#include "WinService.h"

#include "BackupService.h"

WinService* WinService::instance = nullptr;

std::unique_ptr<WinService> WinService::create (const std::string & name)
{
	return std::make_unique<BackupService> (name);
}

int WinService::start ()
{
	instance = this;

	SERVICE_TABLE_ENTRY serviceTable[] =
	{
		{ serviceName, (LPSERVICE_MAIN_FUNCTION)service_main },
		{ NULL, NULL }
	};

	if (StartServiceCtrlDispatcher (serviceTable) == FALSE) {
		DWORD serviceDispatchError = GetLastError ();
		if (serviceDispatchError == ERROR_FAILED_SERVICE_CONTROLLER_CONNECT) {
			onFailedRegistration ();
		}
		else {
			return serviceDispatchError;
		}
	}

	return EXIT_SUCCESS;
}

void __stdcall WinService::service_main (DWORD dwArgc, LPSTR* lpszArgv)
{
	instance->serviceStatusHandle = 
		RegisterServiceCtrlHandlerEx (instance->serviceName, (LPHANDLER_FUNCTION_EX)service_control, 0);

	if (instance->serviceStatusHandle == NULL)
	{
		return;
	}

	// Tell the service controller we are starting
	instance->serviceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
	instance->serviceStatus.dwControlsAccepted = 0;
	instance->serviceStatus.dwCurrentState = SERVICE_START_PENDING;
	instance->serviceStatus.dwWin32ExitCode = 0;
	instance->serviceStatus.dwServiceSpecificExitCode = 0;
	instance->serviceStatus.dwCheckPoint = 0;

	if (SetServiceStatus (instance->serviceStatusHandle, &instance->serviceStatus) == FALSE)
	{
		OutputDebugString ("My Sample Service: ServiceMain: SetServiceStatus returned error");
	}


	// Create a service stop event to wait on later
	instance->serviceStopEvent = INVALID_HANDLE_VALUE;
	instance->serviceStopEvent = CreateEvent (NULL, TRUE, FALSE, NULL);
	if (instance->serviceStopEvent == NULL)
	{
		// Error creating event
		// Tell service controller we are stopped and exit
		instance->serviceStatus.dwControlsAccepted = 0;
		instance->serviceStatus.dwCurrentState = SERVICE_STOPPED;
		instance->serviceStatus.dwWin32ExitCode = GetLastError ();
		instance->serviceStatus.dwCheckPoint = 1;

		if (SetServiceStatus (instance->serviceStatusHandle, &instance->serviceStatus) == FALSE)
		{
			OutputDebugString ("My Sample Service: ServiceMain: SetServiceStatus returned error");
		}
		
		return;
	}



	// Tell the service controller we are started
	instance->serviceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;
	instance->serviceStatus.dwCurrentState = SERVICE_RUNNING;
	instance->serviceStatus.dwWin32ExitCode = 0;
	instance->serviceStatus.dwCheckPoint = 0;

	if (SetServiceStatus (instance->serviceStatusHandle, &instance->serviceStatus) == FALSE)
	{
		OutputDebugString ("My Sample Service: ServiceMain: SetServiceStatus returned error");
	}

	instance->onStarted ();

	// Start a thread that will perform the main task of the service
	HANDLE hThread = CreateThread (NULL, 0, worker_thread, NULL, 0, NULL);

	// Wait until our worker thread exits signaling that the service needs to stop
	WaitForSingleObject (hThread, INFINITE);


	/*
	 * Perform any cleanup tasks
	 */

	CloseHandle (instance->serviceStopEvent);

	// Tell the service controller we are stopped
	instance->serviceStatus.dwControlsAccepted = 0;
	instance->serviceStatus.dwCurrentState = SERVICE_STOPPED;
	instance->serviceStatus.dwWin32ExitCode = 0;
	instance->serviceStatus.dwCheckPoint = 3;

	if (SetServiceStatus (instance->serviceStatusHandle, &instance->serviceStatus) == FALSE)
	{
		OutputDebugString ("My Sample Service: ServiceMain: SetServiceStatus returned error");
	}
}

void __stdcall WinService::service_control (DWORD dwControlCode, DWORD dwEventType, LPVOID lpEventData, LPVOID lpContext)
{
	switch (dwControlCode) {
		case SERVICE_CONTROL_STOP: [[failthrough]]
			instance->onServiceControlStop ();
		case SERVICE_CONTROL_SHUTDOWN: [[failthrough]]
		case SERVICE_CONTROL_PRESHUTDOWN:
			if (instance->serviceStatus.dwCurrentState != SERVICE_RUNNING)
				break;

			/*
			 * Perform tasks necessary to stop the service here
			 */

			instance->serviceStatus.dwControlsAccepted = 0;
			instance->serviceStatus.dwCurrentState = SERVICE_STOP_PENDING;
			instance->serviceStatus.dwWin32ExitCode = 0;
			instance->serviceStatus.dwCheckPoint = 4;

			if (SetServiceStatus (instance->serviceStatusHandle, &instance->serviceStatus) == FALSE)
			{
				OutputDebugString ("My Sample Service: ServiceCtrlHandler: SetServiceStatus returned error");
			}

			// This will signal the worker thread to start shutting down
			SetEvent (instance->serviceStopEvent);
			break;

		case SERVICE_CONTROL_DEVICEEVENT:
			instance->onServiceControlDeviceEvent (dwEventType, lpEventData);
			break;
		default:
			break;

	}
}

DWORD __stdcall WinService::worker_thread (LPVOID lpParam)
{
	while (WaitForSingleObject (instance->serviceStopEvent, 0) != WAIT_OBJECT_0)
	{
		instance->job ();
	}

	return ERROR_SUCCESS;
}
