cmake_minimum_required(VERSION 3.12 FATAL_ERROR)
project ("autobackup" CXX C)

set_property (GLOBAL PROPERTY USE_FOLDERS ON)

set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DSECURITY_WIN32")

if (MSVC)  
	set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4820 /wd4710 /wd4668")  
	set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DNOMINMAX")  
	set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /bigobj")  
	set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /utf-8")  
	set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHa")   
	option(ENABLE_STATIC_LINK "Build with /MT" ON)  
	mark_as_advanced(ENABLE_STATIC_LINK)  
	if (ENABLE_STATIC_LINK STREQUAL ON)   
		set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")   
		set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} /MT")   
		set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")   
		set(CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL} /MT")  
	endif () 
endif ()

set (LIBS "")

set (SRC_FILES 
	src/main.cpp
	#src/Platform.h
	#src/Utils.h
	#src/Timer.h src/Timer.cpp
	#src/DriveInfo.h src/DriveInfo.cpp
	#src/WinService.h src/WinService.cpp
	#src/BackupService.h src/BackupService.cpp
	#src/StartupOptions.h src/StartupOptions.cpp

	src/zip/FileZip.h src/zip/FileZip.cpp
	)

add_subdirectory (vendor)

add_executable(autobackup ${SRC_FILES})
# mINI
# header only
target_include_directories(autobackup PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/vendor/mINI/src)

# libusb 
# header only
target_include_directories(autobackup PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/vendor/libusb/libusb)

# zlib 
find_package (minizip-ng CONFIG REQUIRED)
if (minizip-ng_FOUND)
	target_link_libraries(autobackup PRIVATE MINIZIP::minizip-ng)
endif ()









