cmake_minimum_required(VERSION 3.12)

include(FetchContent)

FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.11.2/json.tar.xz)
FetchContent_MakeAvailable(json)

add_executable(DownloaderDemo
  "src/main.cpp"
 )

target_compile_features(DownloaderDemo PRIVATE cxx_std_17)

target_include_directories(DownloaderLib 
        PUBLIC ../Downloader/src)

target_link_libraries(DownloaderDemo
    PUBLIC DownloaderLib::DownloaderLib
    PRIVATE nlohmann_json::nlohmann_json
)

if(MSVC)
  target_compile_options(DownloaderDemo PRIVATE /W3 /WX)
else()
  target_compile_options(DownloaderDemo PRIVATE -Wall -Wextra -pedantic -Werror)
endif()


install(TARGETS DownloaderDemo
    CONFIGURATIONS Debug
    DESTINATION "${CMAKE_BINARY_DIR}/Debug"
)

install(TARGETS DownloaderDemo
    CONFIGURATIONS Release
    DESTINATION "${CMAKE_BINARY_DIR}/Release"
)

