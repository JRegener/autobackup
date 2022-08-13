#pragma once

#include <condition_variable>
#include <mutex>

#include "WinService.h"
#include "Timer.h"
#include "StartupOptions.h"


class BackupService : public WinService {
public:
	BackupService (const std::string & name) : 
		WinService(name), stopped(false) {}

private:
	virtual void job () override;
	
	virtual void onStarted () override;

	virtual void onServiceControlStop () override;

	virtual void onServiceControlDeviceEvent (DWORD dwEventType, LPVOID lpEventData) override;


	void doBackup (const Archived& source, const std::map<KeyFile, Destination>& destinations);

	void backupToArrivedDevice ();

private:
	bool stopped;
	std::condition_variable cv;
	std::mutex mutex;
	std::vector<Timer> timers;

	HDEVNOTIFY hDevNotify;

	std::map<KeyFile, Archived> sources;
	std::map<KeyFile, Destination> destinations;

};