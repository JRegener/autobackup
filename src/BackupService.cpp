#include "BackupService.h"

#include <Dbt.h>

void BackupService::job ()
{
	// parse ini
	// get source 
	// get destination


	for (auto& src : sources) {
		auto& key = src.first;
		auto& value = src.second;

		if (value.time.mode != TimerOptions::Mode::Disabled) {
			Timer::Mode mode;

			switch (value.time.mode) {
				case TimerOptions::Mode::Loop:
					mode = Timer::Mode::For;
					break;
				case TimerOptions::Mode::Exact:
					mode = Timer::Mode::For;
					break;
			}

			Timer timer ([this, &value, &destinations]() {

				doBackup (value, destinations);

				},
				mode, value.time.seconds, true);

			timers.push_back (std::move (timer));
		}


		for (auto& dest : destinations) {
			auto& value = dest.second;
			if (value.checkConnection) {
				// enable device listener
				// send lambda with backup function
			}
		}

		while (!stopped) {
			std::unique_lock lock (mutex);
			cv.wait (lock);
		}
	}
}

void BackupService::onStarted ()
{
	DEV_BROADCAST_DEVICEINTERFACE NotificationFilter = { 0 };
	NotificationFilter.dbcc_size = sizeof (NotificationFilter);
	NotificationFilter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;

	hDevNotify = RegisterDeviceNotificationA (
		serviceStatusHandle,
		&NotificationFilter,
		DEVICE_NOTIFY_SERVICE_HANDLE | DEVICE_NOTIFY_ALL_INTERFACE_CLASSES);

	if (hDevNotify == nullptr)
	{
		OutputDebugString ("Error RegisterDeviceNotificationA");
		return;
	}
}

void BackupService::onServiceControlStop ()
{
	if (hDevNotify) UnregisterDeviceNotification (hDevNotify);

	std::unique_lock lock (mutex);
	stopped = true;
	cv.notify_one ();
}

void BackupService::onServiceControlDeviceEvent (DWORD dwEventType, LPVOID lpEventData)
{
	/*
	* PDEV_BROADCAST_HDR lpdb = (PDEV_BROADCAST_HDR)lpEventData;
	            if (lpdb->dbch_devicetype == DBT_DEVTYP_VOLUME)
            {
                  // Get Information about the usb device inserted
            }
	*/
	switch (dwEventType)
	{
		case DBT_DEVICEREMOVECOMPLETE:
			OutputDebugString ("Device Removal");
			break;
		case DBT_DEVICEARRIVAL:
			OutputDebugString ("Device Arrival");
			PDEV_BROADCAST_HDR lpdb = (PDEV_BROADCAST_HDR)lpEventData;
			/* check device type */
			switch (lpdb->dbch_devicetype) {
				case DBT_DEVTYP_VOLUME:
					
					break;
			}
			break;
		case DBT_DEVNODES_CHANGED:
			OutputDebugString ("Device Nodes Changed");
			break;
	}
}

void BackupService::doBackup (const Archived& source, const std::map<KeyFile, Destination>& destinations)
{
	for (auto& srcDrive : source.paths) {
		auto srcPath = srcDrive.second.extractPath ();

		for (auto& dest : destinations) {
			for (auto& drive : dest.second.paths) {
				auto destPath = drive.second.extractPath ();

				zip::backup (srcPath, destPath);
			}
		}
	}
}

void BackupService::backupToArrivedDevice ()
{
	for (auto& dest : destinations) {
		auto& key = dest.first;
		auto& value = dest.second;

		if (value.checkConnection) {
			if (DriveInfo::checkConnected (value.path)) {

			}
		}
	}
}

