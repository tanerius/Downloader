cmake_minimum_required(VERSION 3.12)

add_executable(DownloaderDemo
  "src/main.cpp"
 )

target_compile_features(DownloaderDemo PRIVATE cxx_std_17)

target_link_libraries(DownloaderDemo
    PUBLIC DownloaderLib::DownloaderLib
)

if(MSVC)
  target_compile_options(DownloaderDemo PRIVATE /W4 /WX)
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

