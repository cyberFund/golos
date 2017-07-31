include(${GOLOS_ROOT_DIR}/cmake/FindLibcds.cmake)

locate_libcds()

if (NOT ${LIBCDS_IS_BUILT})
    add_subdirectory(libcds)
endif()

add_subdirectory(fc)
add_subdirectory(schema)
add_subdirectory(chainbase)
add_subdirectory(chain)
add_subdirectory(protocol)
add_subdirectory(network)
add_subdirectory(utilities)
add_subdirectory(application)
add_subdirectory(plugins)
add_subdirectory(wallet)
add_subdirectory(manifest)
