function(find_libcds_local_headers)
    unset (libcds_INCLUDE_DIRS CACHE)
    find_path(libcds_INCLUDE_DIRS NAMES cds
            HINTS   ${CMAKE_INSTALL_PREFIX}/include
            PATH_SUFFIXES ${ARGN} NO_DEFAULT_PATH
            )
endfunction()

function(find_libcds_local_so_file)
    unset(libcds_SO_FILE_PATH CACHE)
    find_path(libcds_SO_FILE_PATH NAMES libcds_d.so
            HINTS   ${CMAKE_INSTALL_PREFIX}/lib64
            ${CMAKE_INSTALL_PREFIX}/lib
            PATH_SUFFIXES ${ARGN} NO_DEFAULT_PATH
            )
endfunction()

function(find_libcds_submodule_headers)
    unset (libcds_INCLUDE_DIRS CACHE)
    find_path(libcds_INCLUDE_DIRS NAMES cds
            HINTS   ${GOLOS_ROOT_DIR}/libraries/libcds
            PATH_SUFFIXES ${ARGN} NO_DEFAULT_PATH
            )
endfunction()

function(find_libcds_submodule_so_file)
    unset(libcds_SO_FILE_PATH CACHE)
    find_path(libcds_SO_FILE_PATH NAMES libcds_d.so
            HINTS   ${GOLOS_ROOT_DIR}/libraries/libcds/bin
            ${CMAKE_INSTALL_PREFIX}/lib64
            ${CMAKE_INSTALL_PREFIX}/lib
            PATH_SUFFIXES ${ARGN} NO_DEFAULT_PATH
            )
endfunction()

function(locate_libcds)
    if (libcds_INCLUDE_DIRS AND libcds_SO_FILE_PATH)
        set (LIBCDS_IS_BUILT TRUE PARENT_SCOPE)
        return()
    endif()

    unset(libcds_SO_FILE_PATH CACHE)
    unset (libcds_INCLUDE_DIRS CACHE)

    find_libcds_local_headers()
    find_libcds_local_so_file()

    if(NOT libcds_INCLUDE_DIRS OR NOT libcds_SO_FILE_PATH)
        set (LIBCDS_IS_BUILT FALSE PARENT_SCOPE)
    else()
        message(STATUS "Found libcds. PATH: ${libcds_INCLUDE_DIRS}")
        message(STATUS "Found libcds_d.so PATH: ${libcds_SO_FILE_PATH}")
        set (LIBCDS_IS_BUILT TRUE PARENT_SCOPE)
    endif()
endfunction()
