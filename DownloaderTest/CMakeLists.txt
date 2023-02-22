cmake_minimum_required(VERSION 3.12)

include(FetchContent)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        v1.13.0
)
FetchContent_MakeAvailable(googletest)
add_library(GTest::GTest INTERFACE IMPORTED)

target_link_libraries(GTest::GTest INTERFACE gtest_main)

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
    PRIVATE GTest::GTest
)

if(MSVC)
  target_compile_options(DownloaderTest PRIVATE /W3 /WX)
else()
  target_compile_options(DownloaderTest PRIVATE -Wall -Wextra -pedantic -Werror)
endif()

add_test(DownloaderGTest DownloaderTest)

install(TARGETS DownloaderTest
    CONFIGURATIONS Debug
    DESTINATION "${CMAKE_BINARY_DIR}/Install/Debug"
)

install(TARGETS DownloaderTest
    CONFIGURATIONS Release
    DESTINATION "${CMAKE_BINARY_DIR}/Install/Release"
)

