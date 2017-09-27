set(CURRENT_TARGET chain)

list(APPEND ${CURRENT_TARGET}_HEADERS
     include/steemit/chain/account_evaluator.hpp
     include/steemit/chain/account_object.hpp
     include/steemit/chain/asset_object.hpp
     include/steemit/chain/asset_evaluator.hpp
     include/steemit/chain/block_log.hpp
     include/steemit/chain/block_summary_object.hpp
     include/steemit/chain/comment_object.hpp
     include/steemit/chain/compound.hpp
     include/steemit/chain/custom_operation_interpreter.hpp
     include/steemit/chain/custom_evaluator.hpp
     include/steemit/chain/database.hpp
     include/steemit/chain/exceptions/database_exceptions.hpp
     include/steemit/chain/db_with.hpp
     include/steemit/chain/escrow_evaluator.hpp
     include/steemit/chain/evaluator.hpp
     include/steemit/chain/evaluator_registry.hpp
     include/steemit/chain/fork_database.hpp
     include/steemit/chain/generic_custom_operation_interpreter.hpp
     include/steemit/chain/global_property_object.hpp
     include/steemit/chain/history_object.hpp
     include/steemit/chain/hardfork_object.hpp
     include/steemit/chain/immutable_chain_parameters.hpp
     include/steemit/chain/market_evaluator.hpp
     include/steemit/chain/market_object.hpp
     include/steemit/chain/node_property_object.hpp
     include/steemit/chain/operation_history_object.hpp
     include/steemit/chain/operation_notification.hpp
     include/steemit/chain/proposal_evaluator.hpp
     include/steemit/chain/proposal_object.hpp
     include/steemit/chain/shared_authority.hpp
     include/steemit/chain/shared_db_merkle.hpp
     include/steemit/chain/steem_evaluator.hpp
     include/steemit/chain/steem_object_types.hpp
     include/steemit/chain/steem_objects.hpp
     include/steemit/chain/transfer_evaluator.hpp
     include/steemit/chain/transaction_evaluation_state.hpp
     include/steemit/chain/transaction_object.hpp
     include/steemit/chain/witness_objects.hpp
     include/steemit/chain/witness_schedule.hpp

     include/steemit/chain/utilities/asset.hpp
     include/steemit/chain/utilities/reward.hpp
     include/steemit/chain/utilities/uint256.hpp
     )

list(APPEND ${CURRENT_TARGET}_SOURCES
     account_evaluator.cpp
     account_object.cpp
     asset_evaluator.cpp
     asset_object.cpp
     block_log.cpp
     custom_evaluator.cpp
     database.cpp
     escrow_evaluator.cpp
     fork_database.cpp
     market_evaluator.cpp
     shared_authority.cpp
     steem_evaluator.cpp
     proposal_evaluator.cpp
     steem_objects.cpp
     transfer_evaluator.cpp
     proposal_object.cpp
     witness_schedule.cpp

     utilities/reward.cpp
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

add_dependencies(golos_${CURRENT_TARGET} golos::protocol golos::version)

target_link_libraries(golos_${CURRENT_TARGET} golos::protocol golos::version fc chainbase graphene_schema ${PATCH_MERGE_LIB})
target_include_directories(golos_${CURRENT_TARGET}
                           PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" "${CMAKE_CURRENT_SOURCE_DIR}/../../version/include" "${CMAKE_CURRENT_BINARY_DIR}/include" "${CMAKE_CURRENT_BINARY_DIR}/../../version/include")

if(MSVC)
    set_source_files_properties(database.cpp PROPERTIES COMPILE_FLAGS "/bigobj")
endif(MSVC)

install(TARGETS
        golos_${CURRENT_TARGET}

        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        )

install(FILES ${${CURRENT_TARGET}_HEADERS} DESTINATION "include/steemit/${CURRENT_TARGET}")
