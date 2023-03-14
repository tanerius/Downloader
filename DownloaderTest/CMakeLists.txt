cmake_minimum_required(VERSION 3.21)

if(${DOWNLOADER_LINK_STATICALLY})
    set(gtest_force_shared_crt off)
else()
    set(gtest_force_shared_crt on)
endif()

include(FetchContent)

FetchContent_Declare(googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG main)
FetchContent_MakeAvailable(googletest)

enable_testing()

FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.11.2/json.tar.xz)
FetchContent_MakeAvailable(json)

add_executable(DownloaderTest
    "src/main.cpp"
    )

target_compile_features(DownloaderTest PRIVATE cxx_std_17)

target_include_directories(DownloaderTest 
        PUBLIC ../Downloader/src)

target_link_libraries(DownloaderTest
    PUBLIC EZResume::EZResume
    PRIVATE nlohmann_json::nlohmann_json
    PRIVATE gtest
)

if(MSVC)
    target_compile_options(DownloaderTest PRIVATE /W3 /WX)
else()
    target_compile_options(DownloaderTest PRIVATE -Wall -Wextra -pedantic -Werror)
endif()

add_test(NAME DownloaderTest
    COMMAND DownloaderTest
    )

if(NOT DOWNLOADER_LINK_STATICALLY)
    if(${EZResume_PLATFORM} STREQUAL "Win64")
        add_custom_command(
            TARGET DownloaderTest POST_BUILD 
            COMMAND $<$<CONFIG:Debug,Release>:${CMAKE_COMMAND}> $<$<CONFIG:Debug,Release>:-E> $<$<CONFIG:Debug,Release>:copy_if_different> $<$<CONFIG:Debug,Release>:${DOWNLOADER_BIN_FOLDER}/$<CONFIG>/EZResume.dll> $<TARGET_FILE_DIR:DownloaderTest>)
    elseif(${EZResume_PLATFORM} STREQUAL "Apple")
        add_custom_command(
            TARGET DownloaderTest POST_BUILD 
            COMMAND ${CMAKE_COMMAND} -E copy_if_different ${DOWNLOADER_BIN_FOLDER}/libEZResume.dylib $<TARGET_FILE_DIR:DownloaderTest>)
    endif()
endif()


#install(TARGETS DownloaderTest
#    CONFIGURATIONS Debug
#    DESTINATION "${CMAKE_BINARY_DIR}/Install/Debug"
#)

#install(TARGETS DownloaderTest
#    CONFIGURATIONS Release
#    DESTINATION "${CMAKE_BINARY_DIR}/Install/Release"
#)

