#pragma once

#include "Utils.h"

#include <Windows.h>
#include <string>
#include <filesystem>
#include <functional>

class DriveInfo;

class DrivePath {
	friend class DriveInfo;
public:
	DrivePath (const std::wstring& value) { init (value); }

	std::wstring getName () const { return name; }

	std::wstring getPath () const { return path; }

private:

	void init (const std::wstring& value) {
		std::vector<std::wstring> tokens = utils::split (value, '|');
		if (tokens.size () >= 2)
			name = tokens.front ();
		path = tokens.back ();
	}

private:
	std::wstring name;
	std::wstring path;
};


class DriveInfo {
public:
	static bool checkConnected (const DrivePath& path);

	static std::filesystem::path extractFullPath (const DrivePath& path);
};