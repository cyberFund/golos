set(CURRENT_TARGET protocol)

list(APPEND ${CURRENT_TARGET}_HEADERS
        include/steemit/protocol/asset.hpp
        include/steemit/protocol/asset_operations.hpp
        include/steemit/protocol/authority.hpp
        include/steemit/protocol/base.hpp
        include/steemit/protocol/block.hpp
        include/steemit/protocol/block_header.hpp
        include/steemit/protocol/config.hpp
        include/steemit/protocol/exceptions.hpp
        include/steemit/protocol/get_config.hpp
        include/steemit/protocol/market_operations.hpp
        include/steemit/protocol/market_virtual_operations.hpp
        include/steemit/protocol/operation_utilities.hpp
        include/steemit/protocol/operation_utilities_impl.hpp
        include/steemit/protocol/operations.hpp
        include/steemit/protocol/protocol.hpp
        include/steemit/protocol/sign_state.hpp
        include/steemit/protocol/steem_operations.hpp
        include/steemit/protocol/steem_virtual_operations.hpp
        include/steemit/protocol/transaction.hpp
        include/steemit/protocol/types.hpp
        include/steemit/protocol/version.hpp
        )

list(APPEND ${CURRENT_TARGET}_SOURCES
        asset.cpp
        asset_operations.cpp
        authority.cpp
        block.cpp
        get_config.cpp
        market_operations.cpp
        operation_utilities_impl.cpp
        operations.cpp
        sign_state.cpp
        steem_operations.cpp
        transaction.cpp
        types.cpp
        version.cpp
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

target_link_libraries(golos_${CURRENT_TARGET} fc)
target_include_directories(golos_${CURRENT_TARGET}
        PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" "${CMAKE_CURRENT_BINARY_DIR}/include")

install(TARGETS
        golos_${CURRENT_TARGET}

        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        )

install(FILES ${${CURRENT_TARGET}_HEADERS} DESTINATION "include/steemit/${CURRENT_TARGET}")