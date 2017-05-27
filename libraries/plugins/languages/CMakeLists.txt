file(GLOB HEADERS "include/steemit/languages/*.hpp")


if(BUILD_SHARED_LIBRARIES)
    add_library(golos_languages SHARED
            languages_plugin.cpp)
else()
    add_library(golos_languages STATIC
            languages_plugin.cpp)
endif()



target_link_libraries(golos_languages golos_chain golos_protocol golos::application)
target_include_directories(golos_languages
        PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

install(TARGETS
        golos_languages

        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        )
