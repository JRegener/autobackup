#if 0
#include <iostream>
#include <functional>
#include <filesystem>
#include <fstream>
#include <thread>
#include <chrono>
#include <map>
#include <condition_variable>
#include <mutex>
#include <type_traits>
#include <ctime>

#include <mini/ini.h>

#include "Timer.h"
#include "DriveInfo.h"
#include "BackupService.h"

/*

Usage

1. Settings file which contains list of files and directories needs to be backuped
2. Also this file contains information of time then backup will be run
3. Time can be set for all directories and files, and also for each of them
4. This file contains destinarion directory or disk for backup
5. Program can check on demand (set in settings file) was file/directory changed before backup
6. Program check (on demand (settings file)) when destination external storage was connected to PC
   do backup right away or only by timeout
7. Backup file is archived file in .zip extension creating by this program (using zlib library)
8. Settings file updated before every backup and after directional command to service

*/

/*
.ini structure

[source]
file=/path/to/file
file=/second/path/to/file
[source_options]
[destination]
[destination_options]
[time_options]
file=00:20:00 loop
file2=12:00:00 exact
file3=12:00:00 disabled

*/


int main (int argc, char* argv[]) {
	
	DrivePath drive (L"KINGSTON|\\");
	auto path = drive.initPath ();

	//std::unique_ptr<WinService> service (WinService::create ("Auto backup service"));
	//service->start ();

}

#endif


#if 0
#include <iostream>
#include <Windows.h>
#include <Dbt.h>
#include <tchar.h>

#include <initguid.h>
#include <usbiodef.h>

LRESULT CALLBACK WndProc (HWND hWnd, UINT uiMsg, WPARAM wParam, LPARAM lParam)
{
	if (uiMsg == WM_DEVICECHANGE)
	{
		MessageBox (NULL, TEXT ("WM_DEVICECHANGE"), TEXT ("WndProc"), MB_OK);
		return 0;
	}

	return DefWindowProc (hWnd, uiMsg, wParam, lParam);
}

int _tmain (int argc, _TCHAR* argv[])
{
	HINSTANCE hInstance = reinterpret_cast<HINSTANCE>(GetModuleHandle (NULL));

	WNDCLASS wndClass = { 0 };
	wndClass.lpfnWndProc = &WndProc;
	wndClass.lpszClassName = TEXT ("lua");
	wndClass.hInstance = hInstance;

	if (RegisterClass (&wndClass))
	{
		HWND lua = CreateWindow (
			wndClass.lpszClassName, 
			NULL, 
			0, 
			CW_USEDEFAULT, 
			CW_USEDEFAULT, 
			CW_USEDEFAULT, 
			CW_USEDEFAULT, 
			NULL, 
			NULL, 
			hInstance, 
			NULL);
		
		if (lua != NULL)
		{
			DEV_BROADCAST_DEVICEINTERFACE NotificationFilter = { 0 };
			NotificationFilter.dbcc_size = sizeof (NotificationFilter);
			NotificationFilter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
			NotificationFilter.dbcc_classguid = GUID_CLASS_USB_DEVICE;

			HDEVNOTIFY hVolNotify = RegisterDeviceNotification (
				lua, 
				&NotificationFilter, 
				DEVICE_NOTIFY_WINDOW_HANDLE);

			if (hVoslNotify != NULL)
			{
				MSG msg;
				while (GetMessage (&msg, NULL, 0, 0) > 0)
				{
					TranslateMessage (&msg);
					DispatchMessage (&msg);
				}

				UnregisterDeviceNotification (hVolNotify);
			}
			else {
				std::cout << GetLastError ();
			}

			DestroyWindow (lua);
		}

		UnregisterClass (wndClass.lpszClassName, hInstance);
	}

	return 0;
}

#endif

#include <Windows.h>
#include "zip/FileZip.h"

int main (int argc, char** argv) {
	using namespace autobackup::zip;
	FileZip zipper(u8"тестовый.zip");
	zipper.add (L"C:\\Disk\\test", L"C:\\Disk\\test");


	return 0;
}