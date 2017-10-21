set(CURRENT_TARGET chain)

list(APPEND ${CURRENT_TARGET}_HEADERS
     include/steemit/chain/evaluators/account_evaluator.hpp
     include/steemit/chain/evaluators/account_evaluator.tpp
     include/steemit/chain/objects/account_object.hpp
     include/steemit/chain/objects/asset_object.hpp
     include/steemit/chain/evaluators/asset_evaluator.hpp
     include/steemit/chain/evaluators/asset_evaluator.tpp
     include/steemit/chain/block_log.hpp
     include/steemit/chain/objects/block_summary_object.hpp
     include/steemit/chain/objects/comment_object.hpp
     include/steemit/chain/compound.hpp
     include/steemit/chain/custom_operation_interpreter.hpp
     include/steemit/chain/evaluators/custom_evaluator.hpp
     include/steemit/chain/database.hpp
     include/steemit/chain/database_exceptions.hpp
     include/steemit/chain/db_with.hpp
     include/steemit/chain/evaluators/escrow_evaluator.hpp
     include/steemit/chain/evaluators/escrow_evaluator.tpp
     include/steemit/chain/evaluator.hpp
     include/steemit/chain/evaluator_registry.hpp
     include/steemit/chain/fork_database.hpp
     include/steemit/chain/generic_custom_operation_interpreter.hpp
     include/steemit/chain/objects/global_property_object.hpp
     include/steemit/chain/objects/history_object.hpp
     include/steemit/chain/objects/hardfork_object.hpp
     include/steemit/chain/immutable_chain_parameters.hpp
     include/steemit/chain/evaluators/market_evaluator.hpp
     include/steemit/chain/evaluators/market_evaluator.tpp
     include/steemit/chain/objects/market_object.hpp
     include/steemit/chain/objects/node_property_object.hpp
     include/steemit/chain/objects/operation_history_object.hpp
     include/steemit/chain/operation_notification.hpp
     include/steemit/chain/evaluators/proposal_evaluator.hpp
     include/steemit/chain/evaluators/proposal_evaluator.tpp
     include/steemit/chain/objects/proposal_object.hpp
     include/steemit/chain/shared_authority.hpp
     include/steemit/chain/shared_db_merkle.hpp
     include/steemit/chain/evaluators/steem_evaluator.hpp
     include/steemit/chain/evaluators/steem_evaluator.tpp
     include/steemit/chain/steem_object_types.hpp
     include/steemit/chain/objects/steem_objects.hpp
     include/steemit/chain/evaluators/transfer_evaluator.hpp
     include/steemit/chain/evaluators/transfer_evaluator.tpp
     include/steemit/chain/transaction_evaluation_state.hpp
     include/steemit/chain/objects/transaction_object.hpp
     include/steemit/chain/objects/witness_object.hpp
     include/steemit/chain/witness_schedule.hpp
     include/steemit/chain/evaluators/witness_evaluator.hpp
     include/steemit/chain/evaluators/witness_evaluator.tpp

     include/steemit/chain/utilities/asset.hpp
     include/steemit/chain/utilities/reward.hpp
     include/steemit/chain/utilities/uint256.hpp
     )

list(APPEND ${CURRENT_TARGET}_SOURCES
     evaluators/account_evaluator.cpp
     objects/account_object.cpp
     evaluators/asset_evaluator.cpp
     objects/asset_object.cpp
     block_log.cpp
     evaluators/custom_evaluator.cpp
     database.cpp
     evaluators/escrow_evaluator.cpp
     fork_database.cpp
     evaluators/market_evaluator.cpp
     shared_authority.cpp
     evaluators/steem_evaluator.cpp
     evaluators/proposal_evaluator.cpp
     objects/steem_objects.cpp
     evaluators/transfer_evaluator.cpp
     objects/proposal_object.cpp
     witness_schedule.cpp
     evaluators/witness_evaluator.cpp

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
