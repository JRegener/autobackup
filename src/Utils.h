#pragma once

#include <vector>
#include <string>

#if _WIN32
#include <Windows.h>
#endif

namespace utils {

template<typename Str>
static std::vector<Str> split (const Str& value, char separator) {
	std::vector<Str> result;

	size_t begin = 0;

	size_t pos = value.find (separator, begin);
	while (pos != value.npos) {
		result.emplace_back (value.substr (begin, pos));
		begin = pos + 1;
		pos = value.find (separator, begin);
	}

	result.emplace_back (value.substr (begin, pos));

	return result;
}

#if _WIN32
static std::string wstringToString (const std::wstring& wstr) {
	if (wstr.empty ()) return std::string ();
	int sizeNeeded = WideCharToMultiByte (CP_UTF8, 0, &wstr[0], (int)wstr.size (), NULL, 0, NULL, NULL);
	std::string strTo (sizeNeeded, 0);
	WideCharToMultiByte (CP_UTF8, 0, &wstr[0], (int)wstr.size (), &strTo[0], sizeNeeded, NULL, NULL);
	return strTo;
}

static std::wstring stringToWString (const std::string& str) {
	if (str.empty ()) return std::wstring ();
	int sizeNeeded = MultiByteToWideChar (CP_UTF8, 0, &str[0], (int)str.size (), NULL, 0);
	std::wstring strTo (sizeNeeded, 0);
	MultiByteToWideChar (CP_UTF8, 0, &str[0], (int)str.size (), &strTo[0], sizeNeeded);
	return strTo;
}
#else 

#endif

}