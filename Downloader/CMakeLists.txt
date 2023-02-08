cmake_minimum_required(VERSION 3.12)

set(DL_SRC_PATH "src")


file(GLOB_RECURSE DL_SRC_LIST
            LIST_DIRECTORIES false 
            ${DL_SRC_PATH}/*
        )

add_library(DownloaderLib STATIC 
        ${DL_SRC_LIST}
)

target_compile_features(DownloaderLib PRIVATE cxx_std_17)

target_include_directories(DownloaderLib 
    PUBLIC src
    PUBLIC src/pch
    PUBLIC src/include)

target_link_libraries(DownloaderLib
    PUBLIC CURL::libcurl
)

if(MSVC)
  target_compile_options(DownloaderLib PRIVATE /W4 /WX)
else()
  target_compile_options(DownloaderLib PRIVATE -Wall -Wextra -pedantic -Werror)
endif()

add_library(DownloaderLib::DownloaderLib ALIAS DownloaderLib)

install(TARGETS DownloaderLib
    CONFIGURATIONS Debug
    DESTINATION "${CMAKE_BINARY_DIR}/Debug"
)

install(TARGETS DownloaderLib
    CONFIGURATIONS Release
    DESTINATION "${CMAKE_BINARY_DIR}/Release"
)

