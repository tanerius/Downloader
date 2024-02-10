cmake_minimum_required(VERSION 3.21)

project(EZResume 
    VERSION			1.3.0
    DESCRIPTION		"EZResume downloader library"
    LANGUAGES		CXX	C
)

# Include helpers
include(cmake_modules/Helpers.cmake)

set_platform()

OPTION(DOWNLOADER_LINK_STATICALLY		"Build static library"					On)
OPTION(DOWNLOADER_DEBUG					"Enable debugging logs and asserts"	   Off)
OPTION(DOWNLOADER_BUILD_TESTS			"Whether to build the tests"		    On)

set(DOWNLOAD_EXTRACT_TIMESTAMP ON)
set(DOWNLOADER_BIN_FOLDER "${PROJECT_BINARY_DIR}/Downloader")
if(DOWNLOADER_DEBUG)
    set(DEBUG true)
endif()

# Set some super global stuff here
configure_file(Configurables/versions.h.in ${PROJECT_SOURCE_DIR}/Downloader/include/Versions.h @ONLY)

print_environment()

if(${EZResume_PLATFORM} STREQUAL "Win64")
    set(DOWNLOADER_LIBOUTPUTNAME "ezresume")
elseif(${EZResume_PLATFORM} STREQUAL "Apple")
    set(DOWNLOADER_LIBOUTPUTNAME "ezresume_${EZResume_VERSION}")
endif()


# Add all required externals
add_subdirectory(External)

# Add all required files for Downloader
add_subdirectory(Downloader)

# Add Examples
add_subdirectory(Examples/Simple)

# Add DownloaderDemo

if(${DOWNLOADER_BUILD_TESTS})
    add_subdirectory(DownloaderTest)
endif()

