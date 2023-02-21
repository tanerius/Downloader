cmake_minimum_required(VERSION 3.12)

# 1. Download all the submodules and do the update crap
find_package(Git QUIET)
if(GIT_FOUND AND EXISTS "${PROJECT_SOURCE_DIR}/.git")
# Update submodules as needed
    option(GIT_SUBMODULE "Check submodules during build" ON)
    if(GIT_SUBMODULE)
        message(STATUS "Submodule update")
        execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
                        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                        RESULT_VARIABLE GIT_SUBMOD_RESULT)
        if(NOT GIT_SUBMOD_RESULT EQUAL "0")
            message(FATAL_ERROR "git submodule update --init failed with ${GIT_SUBMOD_RESULT}, please checkout submodules")
        endif()
    endif()
endif()

# 2. Check all submodules to make sure theyre there
### curl settings so we dont build unnecessary stuff 

if(WIN32)
    option(CURL_WERROR "Turn compiler warnings into errors" ON)
    option(PICKY_COMPILER "Enable picky compiler options" ON)
    option(BUILD_CURL_EXE "Set to ON to build curl executable." OFF)
    option(BUILD_SHARED_LIBS "Build shared libraries" OFF)
    option(ENABLE_ARES "Set to ON to enable c-ares support" OFF)
    option(CURL_STATIC_CRT "Set to ON to build libcurl with static CRT on Windows (/MT)." OFF)
    option(ENABLE_INET_PTON "Set to OFF to prevent usage of inet_pton when building against modern SDKs while still requiring compatibility with older Windows versions, such as Windows XP, Windows Server 2003 etc." ON)
    option(ENABLE_UNICODE "Set to ON to use the Unicode version of the Windows API functions" ON)
    option(CURL_USE_SCHANNEL "enable Windows native SSL/TLS" ON) 
    option(CURL_ENABLE_SSL ON)

    if(NOT EXISTS "${PROJECT_SOURCE_DIR}/External/curl/CMakeLists.txt")
        message(FATAL_ERROR "Submodule curl not downloaded")
    else()
        add_subdirectory(curl)
    endif()
else()
    set(CURL_PATH "/usr/local/Cellar/curl/7.88.0")

    find_library(
        LIB_CURL
        NAMES
            curl
        PATHS 
            ${CURL_PATH}/lib)
endif()

### GoogleTest
#set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
#if(NOT EXISTS "${PROJECT_SOURCE_DIR}/External/googletest/CMakeLists.txt")
#    message(FATAL_ERROR "Submodule googletest not downloaded")
#else()
#	add_subdirectory(googletest)
#endif()
