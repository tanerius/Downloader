cmake_minimum_required(VERSION 3.21)

set(DL_SRC_PATH "src")
set(DL_INTERFACE_PATH "include")

message(STATUS ${CMAKE_BINARY_DIR})

# Find the CURL library
find_package(CURL REQUIRED)

file(GLOB_RECURSE DL_SRC_LIST
            LIST_DIRECTORIES false 
            ${DL_SRC_PATH}/*
        )

if(DOWNLOADER_LINK_STATICALLY)
    add_library(EZResume STATIC 
            ${DL_SRC_LIST}
    )
    target_compile_definitions(EZResume PUBLIC -DEZRESUME_STATIC)

    if(MSVC)
        set(CompilerFlags
            CMAKE_CXX_FLAGS
            CMAKE_CXX_FLAGS_DEBUG
            CMAKE_CXX_FLAGS_RELEASE
            CMAKE_CXX_FLAGS_MINSIZEREL
            CMAKE_CXX_FLAGS_RELWITHDEBINFO
            CMAKE_C_FLAGS
            CMAKE_C_FLAGS_DEBUG
            CMAKE_C_FLAGS_RELEASE
            CMAKE_C_FLAGS_MINSIZEREL
            CMAKE_C_FLAGS_RELWITHDEBINFO
            )
        foreach(CompilerFlag ${CompilerFlags})
            string(REPLACE "/MD" "/MT" ${CompilerFlag} "${${CompilerFlag}}")
            set(${CompilerFlag} "${${CompilerFlag}}" CACHE STRING "msvc compiler flags" FORCE)
            message("MSVC flags: ${CompilerFlag}:${${CompilerFlag}}")
        endforeach()
    endif()

else()
    add_library(EZResume SHARED 
            ${DL_SRC_LIST}
    )

    if(MSVC)
        set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
        set(BUILD_SHARED_LIBS TRUE)

        set(CompilerFlags
            CMAKE_CXX_FLAGS
            CMAKE_CXX_FLAGS_DEBUG
            CMAKE_CXX_FLAGS_RELEASE
            CMAKE_CXX_FLAGS_MINSIZEREL
            CMAKE_CXX_FLAGS_RELWITHDEBINFO
            CMAKE_C_FLAGS
            CMAKE_C_FLAGS_DEBUG
            CMAKE_C_FLAGS_RELEASE
            CMAKE_C_FLAGS_MINSIZEREL
            CMAKE_C_FLAGS_RELWITHDEBINFO
        )
        foreach(CompilerFlag ${CompilerFlags})
            string(REPLACE "/MT" "/MD" ${CompilerFlag} "${${CompilerFlag}}")
            set(${CompilerFlag} "${${CompilerFlag}}" CACHE STRING "msvc compiler flags" FORCE)
            message("MSVC flags: ${CompilerFlag}:${${CompilerFlag}}")
        endforeach()
    endif()

    target_compile_definitions(EZResume PRIVATE "EZResume_BUILD_EXPORTS")
endif()

set_property(TARGET EZResume PROPERTY CXX_STANDARD 17)
target_compile_features(EZResume PRIVATE cxx_std_17)

set_target_properties(EZResume PROPERTIES 
    PUBLIC_HEADER "${DL_INTERFACE_PATH}/EZResume.h;${DL_INTERFACE_PATH}/Versions.h"
    OUTPUT_NAME ${DOWNLOADER_LIBOUTPUTNAME}
    )

if(WIN32)
    target_include_directories(EZResume 
        PUBLIC src
        public ${DL_INTERFACE_PATH}
    )

    target_link_libraries(EZResume
        ${CURL_LIBRARIES}
    )

    
else()
    target_link_libraries(
        EZResume
            ${CURL_LIBRARIES})

    target_include_directories(
        EZResume 
            PUBLIC ${CURL_INCLUDE_DIR}
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

