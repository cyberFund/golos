set(CURRENT_TARGET application)

list(APPEND CURRENT_TARGET_HEADERS
        include/steemit/app/api.hpp
        include/steemit/app/api_access.hpp
        include/steemit/app/api_context.hpp
        include/steemit/app/application.hpp
        include/steemit/app/applied_operation.hpp
        include/steemit/app/database_api.hpp
        include/steemit/app/impacted.hpp
        include/steemit/app/plugin.hpp
        include/steemit/app/state.hpp
        include/steemit/app/steem_api_objects.hpp
        )

list(APPEND CURRENT_TARGET_SOURCES
        database_api.cpp
        api.cpp
        application.cpp
        impacted.cpp
        plugin.cpp
        )

if(BUILD_SHARED_LIBRARIES)
    add_library(golos_${CURRENT_TARGET} SHARED
            ${CURRENT_TARGET_HEADERS}
            ${CURRENT_TARGET_SOURCES}
            )
else()
    add_library(golos_${CURRENT_TARGET} STATIC
            ${CURRENT_TARGET_HEADERS}
            ${CURRENT_TARGET_SOURCES}
            )
endif()

add_library(golos::${CURRENT_TARGET} ALIAS golos_${CURRENT_TARGET})
set_property(TARGET golos_${CURRENT_TARGET} PROPERTY EXPORT_NAME ${CURRENT_TARGET})

target_link_libraries(golos_${CURRENT_TARGET} golos::chain golos::protocol golos_tags golos_follow golos_mf_plugins fc graphene_net graphene_time graphene_utilities)
target_include_directories(golos_${CURRENT_TARGET}
        PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

if(MSVC)
    set_source_files_properties(application.cpp api.cpp database_api.cpp PROPERTIES COMPILE_FLAGS "/bigobj")
endif(MSVC)

install(TARGETS
        golos_${CURRENT_TARGET}

        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        )
install(FILES ${HEADERS} DESTINATION "include/steemit/application")