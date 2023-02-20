cmake_minimum_required(VERSION 3.12)

set(DL_SRC_PATH "src")
set(DL_INTERFACE_PATH "include")

message(STATUS ${CMAKE_BINARY_DIR})

file(GLOB_RECURSE DL_SRC_LIST
            LIST_DIRECTORIES false 
            ${DL_SRC_PATH}/*
        )

if(${DOWNLOADER_LINK_STATICALLY})
add_library(EZResume STATIC 
        ${DL_SRC_LIST}
)
else()
add_library(EZResume SHARED 
        ${DL_SRC_LIST}
)
endif()

set_property(TARGET EZResume PROPERTY CXX_STANDARD 17)
target_compile_features(EZResume PRIVATE cxx_std_17)

set_target_properties(EZResume PROPERTIES PUBLIC_HEADER "${DL_INTERFACE_PATH}/EZResume.h;${DL_INTERFACE_PATH}/Versions.h")

if(WIN32)
    target_include_directories(EZResume 
        PUBLIC src
        public ${DL_INTERFACE_PATH}
    )

    target_link_libraries(EZResume
        PUBLIC CURL::libcurl
    )
else()
    target_link_libraries(
        EZResume
            ${LIB_CURL})

    target_include_directories(
        EZResume 
            PUBLIC "${CURL_PATH}/include"
            PUBLIC ${DL_INTERFACE_PATH}
    )
endif()

if(MSVC)
  target_compile_options(EZResume PRIVATE /W3 /WX)
else()
  target_compile_options(EZResume PRIVATE -Wall -Wextra -pedantic -Werror)
endif()

add_library(EZResume::EZResume ALIAS EZResume)

install(TARGETS EZResume
    CONFIGURATIONS Debug
    DESTINATION "${CMAKE_BINARY_DIR}/Install/Debug"
    PUBLIC_HEADER DESTINATION "${CMAKE_BINARY_DIR}/Install/Debug/include"
)

install(TARGETS EZResume
    CONFIGURATIONS Release
    DESTINATION "${CMAKE_BINARY_DIR}/Install/Release"
    PUBLIC_HEADER DESTINATION "${CMAKE_BINARY_DIR}/Install/Release/include"
)

