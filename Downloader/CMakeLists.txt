cmake_minimum_required(VERSION 3.12)

set(DL_SRC_PATH "src")
set(DL_INTERFACE_PATH "include")

message(STATUS ${CMAKE_BINARY_DIR})

file(GLOB_RECURSE DL_SRC_LIST
            LIST_DIRECTORIES false 
            ${DL_SRC_PATH}/*
        )

add_library(DownloaderLib STATIC 
        ${DL_SRC_LIST}
)

target_compile_features(DownloaderLib PRIVATE cxx_std_17)

set_target_properties(DownloaderLib PROPERTIES PUBLIC_HEADER "${DL_INTERFACE_PATH}/EZResume.h")

if(WIN32)
    target_include_directories(DownloaderLib 
        PUBLIC src
        public ${DL_INTERFACE_PATH}
    )

    target_link_libraries(DownloaderLib
        PUBLIC CURL::libcurl
    )
else()
    target_link_libraries(
        DownloaderLib
            ${LIB_CURL})

    target_include_directories(
        DownloaderLib 
            PUBLIC "${CURL_PATH}/include"
            PUBLIC ${DL_INTERFACE_PATH}
    )
endif()

if(MSVC)
  target_compile_options(DownloaderLib PRIVATE /W3 /WX)
else()
  target_compile_options(DownloaderLib PRIVATE -Wall -Wextra -pedantic -Werror)
endif()

add_library(DownloaderLib::DownloaderLib ALIAS DownloaderLib)

install(TARGETS DownloaderLib
    CONFIGURATIONS Debug
    DESTINATION "${CMAKE_BINARY_DIR}/Install/Debug"
    PUBLIC_HEADER DESTINATION "${CMAKE_BINARY_DIR}/Install/Debug/include"
)

install(TARGETS DownloaderLib
    CONFIGURATIONS Release
    DESTINATION "${CMAKE_BINARY_DIR}/Install/Release"
    PUBLIC_HEADER DESTINATION "${CMAKE_BINARY_DIR}/Install/Release/include"
)

