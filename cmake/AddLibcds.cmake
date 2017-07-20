function(find_libcds_headers)
    unset (libcds_INCLUDE_DIRS CACHE)
    find_path(libcds_INCLUDE_DIRS NAMES cds libcds
            PATHS   ${GOLOS_ROOT_DIR}/libraries/libcds
            PATH_SUFFIXES ${ARGN} NO_DEFAULT_PATH
            )
endfunction()

function(find_libcds_so_file)
    unset(libcds_SO_FILE_PATH CACHE)
    if (BUILD_GOLOS_TESTNET)
        find_path(libcds_SO_FILE_PATH NAMES libcds_d.so libcds.so.2.3.0
                HINTS   ${GOLOS_ROOT_DIR}/libraries/libcds/bin/
                PATH_SUFFIXES ${ARGN} NO_DEFAULT_PATH
                )
    else()
        find_path(libcds_SO_FILE_PATH NAMES libcds_d.so libcds.so.2.3.0
                HINTS   /usr/local/lib64
                /usr/local/lib
                PATH_SUFFIXES ${ARGN} NO_DEFAULT_PATH
                )
    endif()
endfunction()