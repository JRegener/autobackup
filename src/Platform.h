#pragma once

#include <string>

namespace platform {


#if _WIN32
using string_value = std::wstring;
#else
using string_value = std::string;
#endif

}