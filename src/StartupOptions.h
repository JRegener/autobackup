#pragma once

#include <filesystem>
#include <chrono>
#include <map>
#include <mini/ini.h>

#include "DriveInfo.h"

using KeyFile = std::string;
using DriveKey = std::wstring;

struct TimerOptions;
struct Archived;
struct Destination;

struct TimerOptions {
	std::chrono::seconds seconds{};
	enum class Mode {
		Disabled,
		Loop,
		Exact
	} mode;
};

struct Archived {
	DrivePath path;
	TimerOptions time;
};

struct Destination {
	DrivePath path;
	bool checkConnection = false;
};




class StartupOptions {
	static constexpr const char section_source[] = "source";
	static constexpr const char section_source_options[] = "source_options";
	static constexpr const char section_destination[] = "destination";
	static constexpr const char section_destination_options[] = "destination_options";
	static constexpr const char section_time_options[] = "time_options";

public:
	StartupOptions (const std::filesystem::path& iniPath) :
		iniPath (iniPath) {}

	void processIni () {
		mINI::INIFile iniFile (inifile);
		mINI::INIStructure ini;
		iniFile.read (ini);
		
		parseSectionSource (ini);
		parseSectionSourceOptions (ini);
		parseSectionDestination (ini);
		parseSectionDestinationOptions (ini);
		parseSectionTimeOptions (ini);
	}

private:
	void parseSectionSource (mINI::INIStructure& ini) {
		/* [source] */
		for (auto path : ini[section_source]) {
			const std::string& key = path.first;
			const std::string& value = path.second;

			if (archivedNodes.find (key) != archivedNodes.end ()) {
				auto& paths = archivedNodes[key].paths;
				if (std::find (paths.begin (), paths.end (), value) == paths.end ()) {
					paths.emplace_back (DrivePath (value));
				}
			}
			else {
				ArchivedNode node;
				node.name = key;
				node.paths.emplace_back (DrivePath (value));
				archivedNodes[key] = node;
			}
		}
	}

	void parseSectionSourceOptions (mINI::INIStructure& ini) {
		/* [source_options] */
		for (auto option : ini[section_source_options]) {
			const std::string& key = option.first;
			std::string& value = option.second;

			/* every time then file changed do backup */
			if (key == "check_when_changed") {
				std::vector<std::string> tokens = split (value, ',');
				for (auto name : tokens) {
					if (archivedNodes.find (name) != archivedNodes.end ()) {
						archivedNodes[name].checkChanging = true;
					}
				}
			}
		}

	}

	void parseSectionDestination (mINI::INIStructure& ini) {
		/* [destination] */
		for (auto path : ini[section_destination]) {
			const std::string& key = path.first;
			const std::string& value = path.second;

			if (destinationNodes.find (key) != destinationNodes.end ()) {
				auto& paths = destinationNodes[key].paths;
				if (std::find (paths.begin (), paths.end (), value) == paths.end ()) {
					paths.emplace_back (DrivePath (value));
				}
			}
			else {
				DestinationNode node;
				node.name = key;
				node.paths.emplace_back (DrivePath (value));
				destinationNodes[key] = node;
			}
		}


	}

	void parseSectionDestinationOptions (mINI::INIStructure& ini) {
		/* [destination_options] */
		for (auto option : ini[section_destination_options]) {
			const std::string& key = option.first;
			std::string& value = option.second;

			/* every time then file changed do backup */
			if (key == "check_connected") {
				std::vector<std::string> tokens = split (value, ',');
				for (auto name : tokens) {
					if (destinationNodes.find (name) != destinationNodes.end ()) {
						destinationNodes[name].checkConnected = true;
					}
				}
			}
		}

	}

	void parseSectionTimeOptions (mINI::INIStructure& ini) {
		/* [time_options] */
		for (auto option : ini[section_time_options]) {
			const std::string& key = option.first;
			std::string& value = option.second;

			if (archivedNodes.find (key) != archivedNodes.end ()) {
				std::transform (value.begin (), value.end (), value.begin (), ::tolower);

				std::vector<std::string> tokens = split (value, ' ');

				std::tm tm{};
				std::istringstream iss (tokens.front ());
				iss >> std::get_time (&tm, "%H:%M:%S");

				auto hours = std::chrono::hours (tm.tm_hour);
				auto minutes = std::chrono::minutes (tm.tm_min);
				auto seconds = std::chrono::seconds (tm.tm_sec);

				TimerMode timerMode = TimerMode::Disabled;

				std::string& mode = tokens.back ();
				mode.erase (std::remove_if (mode.begin (), mode.end (), ::isspace), mode.end ());
				if (mode == "loop") timerMode = TimerMode::Loop;
				else if (mode == "exact") timerMode = TimerMode::Exact;

				archivedNodes[key].time.seconds = hours + minutes + seconds;
				archivedNodes[key].time.mode = timerMode;
			}
		}

	}

private:
	
	
	std::filesystem::path iniPath;

};