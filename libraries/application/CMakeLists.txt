set(CURRENT_TARGET application)

list(APPEND ${CURRENT_TARGET}_HEADERS
        include/steemit/application/api.hpp
        include/steemit/application/api_access.hpp
        include/steemit/application/api_context.hpp
        include/steemit/application/application.hpp
        include/steemit/application/applied_operation.hpp
        include/steemit/application/database_api.hpp
        include/steemit/application/impacted.hpp
        include/steemit/application/plugin.hpp
        include/steemit/application/state.hpp
        include/steemit/application/steem_api_objects.hpp
        )

list(APPEND ${CURRENT_TARGET}_SOURCES
        database_api.cpp
        api.cpp
        application.cpp
        impacted.cpp
        plugin.cpp
        )

if(BUILD_SHARED_LIBRARIES)
    add_library(golos_${CURRENT_TARGET} SHARED
            ${${CURRENT_TARGET}_HEADERS}
            ${${CURRENT_TARGET}_SOURCES}
            )
else()
    add_library(golos_${CURRENT_TARGET} STATIC
            ${${CURRENT_TARGET}_HEADERS}
            ${${CURRENT_TARGET}_SOURCES}
            )
endif()

add_library(golos::${CURRENT_TARGET} ALIAS golos_${CURRENT_TARGET})
set_property(TARGET golos_${CURRENT_TARGET} PROPERTY EXPORT_NAME ${CURRENT_TARGET})

target_link_libraries(golos_${CURRENT_TARGET} golos::chain golos::protocol golos::snapshot golos_tags golos_follow golos_mf_plugins fc golos::network graphene_utilities)
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
install(FILES ${${CURRENT_TARGET}_HEADERS} DESTINATION "include/steemit/${CURRENT_TARGET}")