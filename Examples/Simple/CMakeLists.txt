cmake_minimum_required(VERSION 3.21)

add_executable(SimpleExample
    "src/main.cpp")

target_compile_features(SimpleExample PRIVATE cxx_std_17)

target_include_directories(SimpleExample 
        PUBLIC ../Downloader/src)

target_link_libraries(SimpleExample
    PUBLIC EZResume::EZResume
)

if(MSVC)
    target_compile_options(SimpleExample PRIVATE /W3 /WX)
else()
    target_compile_options(SimpleExample PRIVATE -Wall -Wextra -pedantic -Werror)
endif()

if(NOT DOWNLOADER_LINK_STATICALLY)
    if(${EZResume_PLATFORM} STREQUAL "Win64")
        add_custom_command(
            TARGET SimpleExample POST_BUILD 
            COMMAND $<$<CONFIG:Debug,Release>:${CMAKE_COMMAND}> $<$<CONFIG:Debug,Release>:-E> $<$<CONFIG:Debug,Release>:copy_if_different> $<$<CONFIG:Debug,Release>:${DOWNLOADER_BIN_FOLDER}/$<CONFIG>/${DOWNLOADER_LIBOUTPUTNAME}.dll> $<TARGET_FILE_DIR:SimpleExample>)
    elseif(${EZResume_PLATFORM} STREQUAL "Apple")
        add_custom_command(
            TARGET SimpleExample POST_BUILD 
            COMMAND ${CMAKE_COMMAND} -E copy_if_different ${DOWNLOADER_BIN_FOLDER}/lib${DOWNLOADER_LIBOUTPUTNAME}.dylib $<TARGET_FILE_DIR:SimpleExample>)
    endif()
endif()
