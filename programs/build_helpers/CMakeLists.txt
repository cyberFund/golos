add_executable(cat-parts cat-parts.cpp)

if(UNIX AND NOT APPLE)
    set(rt_library rt)
elseif(APPLE)
    list(APPEND PLATFORM_SPECIFIC_LIBS readline)
endif()

# we only actually need Boost, but link against FC for now so we don't duplicate it.
target_link_libraries(cat-parts PRIVATE fc ${CMAKE_DL_LIBS} ${PLATFORM_SPECIFIC_LIBS})
