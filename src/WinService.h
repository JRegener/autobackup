#pragma once

#include <Windows.h>
#include <string>

#include <functional>

class WinService {
public:
	static std::unique_ptr<WinService> create (const std::string& name);

	WinService (const std::string& name) :
		serviceName (const_cast<char*>(name.c_str ())) {}
	int start ();

protected:
	virtual void onFailedRegistration () {}
	virtual void onStarted () {}
	virtual void job () = 0;
	virtual void onServiceControlStop () {}
	virtual void onServiceControlDeviceEvent (DWORD dwEventType, LPVOID lpEventData) {}

private:
	static void WINAPI service_main (DWORD dwArgc, LPSTR* lpszArgv);
	static void WINAPI service_control (DWORD dwControlCode, DWORD dwEventType, LPVOID lpEventData, LPVOID lpContext);
	static DWORD WINAPI worker_thread (LPVOID lpParam);

protected:
	static WinService* instance;

	LPSTR serviceName;

	SERVICE_STATUS serviceStatus{ 0 };
	SERVICE_STATUS_HANDLE serviceStatusHandle;
	HANDLE serviceStopEvent;
};