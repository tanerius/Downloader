function(congifure_prebuilt_lib test_locations url filename dest)
    foreach(ONE_PATH ${test_locations})
        if( NOT EXISTS ${ONE_PATH} )
            set(DO_DL 1)
            break()
        endif()
    endforeach()

    if(DEFINED DO_DL)
        file(GLOB file_list_pbl
            LIST_DIRECTORIES true 
            ${dest}/*
        )

        message(STATUS " * * * Cleaning destination ${dest} except CMakeLists")

        list(REMOVE_ITEM file_list_pbl "${dest}/CMakeLists.txt")
        list(LENGTH file_list_pbl lib_dir_list_size)

        if(NOT lib_dir_list_size EQUAL 0)
            foreach(TO_REMOVE ${file_list_pbl})
                file(REMOVE_RECURSE "${TO_REMOVE}")
            endforeach()
        endif()
    
        message(STATUS " * * * Downloading package from ${url} ...")

        file(DOWNLOAD ${url} ${dest}/${filename} STATUS DL_RESULT)
        list(GET DL_RESULT 0 DL_RESULT_CODE)
    
        if(NOT DL_RESULT_CODE EQUAL 0)
            message(FATAL_ERROR " * * * Failed downloading! Error: ${DL_RESULT}.")
        endif()
    
        message(STATUS " * * * Extracting file ${dest}/${filename} ...")

        file(ARCHIVE_EXTRACT INPUT "${dest}/${filename}"
            DESTINATION "${dest}"
        )

        message(STATUS " * * * Removing file ${dest}/${filename} ...")
        file(REMOVE_RECURSE "${dest}/${filename}")
    else()
        message(STATUS " * * * Package ${url} appears to be set up already.")
        file(REMOVE_RECURSE "${dest}/${filename}")
    endif()
endfunction()


macro(set_farscape_platform)
    if(APPLE)
        # MacOS, iOS, watchOS, tvOS
        set(FARSCAPE_PLATFORM "Apple")
        set(FS_APPLE "Apple")
    elseif(WIN32 OR WIN64)
        # Any windows
        set(FARSCAPE_PLATFORM "Win64")
        set(FS_WIN64 "Win64")
    elseif(UNIX AND NOT APPLE)
        # Nix
        set(FARSCAPE_PLATFORM "Linux")
        set(FS_LINUX "Linux")
    else()
        # Nix
        set(FARSCAPE_PLATFORM "NA")
        set(FS_PLATFORM_NA "Linux")
    endif()
    
endmacro()


function(print_farscape_environment)
    message(STATUS "FARSCAPE_PLATFORM					${FARSCAPE_PLATFORM}")
    message(STATUS "CONFIG_TYPE					    ${CONFIG_TYPE}")
    if(FARSCAPE_LINK_STATICALLY)
        message(STATUS "Farscape linking type				STATIC ")
        add_definitions(-DFARSCAPE_STATIC)
    else()
        message(STATUS "Farscape linking type				DYNAMIC ")
    endif()
    message(STATUS "FARSCAPE_OUTPUT_DIR				${FARSCAPE_OUTPUT_DIR}")
    message(STATUS "FARSCAPE_BINARY_DIR				${FARSCAPE_BINARY_DIR}")
    message(STATUS "FARSCAPE_INTERMEDIATE_DIR			${FARSCAPE_INTERMEDIATE_DIR}")
    message(STATUS "FARSCAPE_INSTALL_DIR				${FARSCAPE_INSTALL_DIR}")
    message(STATUS "CMAKE_BINARY_DIR					${CMAKE_BINARY_DIR}")
    message(STATUS "CMAKE_INSTALL_PREFIX				${CMAKE_INSTALL_PREFIX}")
    message(STATUS "FARSCAPE_EXTERNALS_DIR			${FARSCAPE_EXTERNALS_DIR}")
    message(STATUS "FARSCAPE_ASSIMP_LIB				${FARSCAPE_ASSIMP_LIB}")
    message(STATUS "FARSCAPE_ASSIMP_DLL				${FARSCAPE_ASSIMP_DLL}")
endfunction()


macro(set_farscape_globals)
    message(STATUS "Setting directories...")
    set(FARSCAPE_OUTPUT_DIR "${CMAKE_SOURCE_DIR}/out")
    set(FARSCAPE_BINARY_DIR "${FARSCAPE_OUTPUT_DIR}/bin")
    set(FARSCAPE_BUILD_DIR "${FARSCAPE_OUTPUT_DIR}/build")
    set(FARSCAPE_INTERMEDIATE_DIR "${FARSCAPE_OUTPUT_DIR}/intermediate")
    set(FARSCAPE_INSTALL_DIR "${FARSCAPE_OUTPUT_DIR}/install")
    set(FARSCAPE_EXTERNALS_DIR "${CMAKE_SOURCE_DIR}/external")
    string(TIMESTAMP BUILD_DATE "%Y-%m-%d %H:%M" UTC)
endmacro()
