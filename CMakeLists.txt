cmake_minimum_required(VERSION 3.12)

project(EZResume 
    VERSION			1.0.0
    DESCRIPTION		"EZResume downloader library"
    LANGUAGES		CXX	C
)




# Include helpers
include(cmake_modules/Helpers.cmake)

set_platform()

OPTION(DOWNLOADER_LINK_STATICALLY		"Build static library"					On)
OPTION(DOWNLOADER_DEBUG					"Enable debugging logs and asserts"		On)

# Set some super global stuff here
configure_file(Configurables/versions.h.in ${PROJECT_SOURCE_DIR}/Downloader/include/Versions.h @ONLY)

if(${DOWNLOADER_LINK_STATICALLY})
    message(STATUS "* * * Configuring EZResume for static libs")
else()
    message(STATUS "* * * Configuring EZResume for dynamic libs")
endif()


# Add all required externals
add_subdirectory(External)

# Add all required files for Downloader
add_subdirectory(Downloader)

# Add DownloaderDemo
add_subdirectory(DownloaderDemo)

