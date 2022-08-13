#include "DriveInfo.h"

bool DriveInfo::checkConnected (const DrivePath& path)
{
	WCHAR drivesNames[MAX_PATH];
	DWORD logicalDS = GetLogicalDriveStringsW (GetLogicalDrives (), drivesNames);

	if (!logicalDS) {
		throw std::runtime_error ("Error extracting GetLogicalDriveStringsW");
	}

	if (path.name.empty () && path.path.size () >= 3) {
		std::wstring driveLetter = path.path.substr (0, 3);

		WCHAR letter[4] = { 0 };
		for (size_t i = 0; i < MAX_PATH; i += 4) {
			::memcpy_s (&letter, sizeof (WCHAR) * 4, &drivesNames[i], sizeof (WCHAR) * 4);
			
			if (std::wstring (letter) == driveLetter) {
				return true;
			}
		}
	}
	else {
		WCHAR letter[4] = { 0 };
		for (size_t i = 0; i < MAX_PATH; i += 4) {
			::memcpy_s (&letter, sizeof (WCHAR) * 4, &drivesNames[i], sizeof (WCHAR) * 4);

			WCHAR szVolumeName[MAX_PATH];
			BOOL bSucceeded = GetVolumeInformationW (letter,
				szVolumeName,
				MAX_PATH,
				NULL,
				NULL,
				NULL,
				NULL,
				0);

			if (!bSucceeded) {
				continue;
			}

			std::wstring wstringVolumeName (szVolumeName);
			if (wstringVolumeName == path.name) {
				return true;
			}
		}
	}

	return false;
}

std::filesystem::path DriveInfo::extractFullPath (const DrivePath& path)
{
	if (path.name.empty ()) {
		return std::filesystem::path (path.path);
	}
	else {
		WCHAR drivesNames[MAX_PATH];
		DWORD logicalDS = GetLogicalDriveStringsW (GetLogicalDrives (), drivesNames);

		if (!logicalDS) {
			throw std::runtime_error ("Error extracting GetLogicalDriveStringsW");
		}

		WCHAR letter[4] = { 0 };
		for (size_t i = 0; i < MAX_PATH; i += 4) {
			::memcpy_s (&letter, sizeof (WCHAR) * 4, &drivesNames[i], sizeof (WCHAR) * 4);

			WCHAR szVolumeName[MAX_PATH];
			BOOL bSucceeded = GetVolumeInformationW (letter,
				szVolumeName,
				MAX_PATH,
				NULL,
				NULL,
				NULL,
				NULL,
				0);

			if (!bSucceeded) {
				continue;
			}

			std::wstring wstringVolumeName (szVolumeName);
			if (wstringVolumeName == path.name) {
				return std::filesystem::path (std::wstring (letter) + path.path);
			}
		}
	}
}
