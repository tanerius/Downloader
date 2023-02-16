cmake_minimum_required(VERSION 3.12)

project(Downloader 
    VERSION			0.0.1
    DESCRIPTION		"Generic downloader library"
    LANGUAGES		CXX	C
)

# Include helpers
include(cmake_modules/Helpers.cmake)

OPTION(DOWNLOADER_LINK_STATICALLY		"Build static library"					On)
OPTION(DOWNLOADER_DEBUG					"Enable debugging logs and asserts"		On)

# Set some super global stuff here
configure_file(Configurables/versions.h.in Downloader/src/Versions.h @ONLY)


# Add all required externals
add_subdirectory(External)

# Add all required files for Downloader
add_subdirectory(Downloader)

# Add DownloaderDemo
add_subdirectory(DownloaderDemo)

