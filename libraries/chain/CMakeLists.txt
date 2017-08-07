set(CURRENT_TARGET chain)

if(MSVC)
    set(hardfork_hpp_file "${CMAKE_CURRENT_SOURCE_DIR}/include/steemit/chain/hardfork.hpp")
    add_custom_target(build_hardfork_hpp
            COMMAND cat-parts "${CMAKE_CURRENT_SOURCE_DIR}/hardfork.d" ${hardfork_hpp_file})
    add_dependencies(build_hardfork_hpp cat-parts)
else(MSVC)
    set(hardfork_hpp_file "${CMAKE_CURRENT_BINARY_DIR}/include/steemit/chain/hardfork.hpp")
    add_custom_target(build_hardfork_hpp
            COMMAND "${CMAKE_SOURCE_DIR}/programs/build_helpers/cat_parts.py" "${CMAKE_CURRENT_SOURCE_DIR}/hardfork.d" ${hardfork_hpp_file})
endif(MSVC)

set_source_files_properties("${CMAKE_CURRENT_BINARY_DIR}/include/steemit/chain/hardfork.hpp" PROPERTIES GENERATED TRUE)

list(APPEND CURRENT_TARGET_HEADERS
        include/steemit/chain/chain_objects/steem_object_types.hpp
        include/steemit/chain/chain_objects/steem_objects.hpp
        include/steemit/chain/chain_objects/transaction_object.hpp
        include/steemit/chain/chain_objects/witness_objects.hpp
        include/steemit/chain/chain_objects/block_summary_object.hpp
        include/steemit/chain/chain_objects/account_object.hpp
        include/steemit/chain/chain_objects/global_property_object.hpp
        include/steemit/chain/chain_objects/history_object.hpp

        include/steemit/chain/database/policies/account_policy.hpp
        include/steemit/chain/database/policies/asset_policy.hpp
        include/steemit/chain/database/policies/behaviour_based_policy.hpp
        include/steemit/chain/database/policies/comment_policy.hpp
        include/steemit/chain/database/generic_policy.hpp
        include/steemit/chain/database/policies/order_policy.hpp
        include/steemit/chain/database/policies/reward_policy.hpp
        include/steemit/chain/database/policies/withdrawal_policy.hpp
        include/steemit/chain/database/policies/witness_policy.hpp
        include/steemit/chain/database/policies/witness_schedule_policy.hpp

        include/steemit/chain/database/worker/account_worker.hpp
        include/steemit/chain/database/worker/behaviour_based_worker.hpp
        include/steemit/chain/database/worker/asset_worker.hpp
        include/steemit/chain/database/worker/reward_worker.hpp
        include/steemit/chain/database/worker/witness_worker.hpp

        include/steemit/chain/database/database.hpp
        include/steemit/chain/database/database_basic.hpp
        include/steemit/chain/database/database_exceptions.hpp
        include/steemit/chain/database/database_police.hpp
        include/steemit/chain/database/database_worker.hpp

        include/steemit/chain/dynamic_extension/smart_worker.hpp
        include/steemit/chain/dynamic_extension/worker_storage.hpp
        include/steemit/chain/dynamic_extension/view.hpp
        include/steemit/chain/dynamic_extension/worker.hpp

        include/steemit/chain/evaluators/evaluator.hpp
        include/steemit/chain/evaluators/evaluator_registry.hpp

        include/steemit/chain/evaluators/account_create_evaluator.hpp
        include/steemit/chain/evaluators/account_create_with_delegation_evaluator.hpp
        include/steemit/chain/evaluators/account_update_evaluator.hpp
        include/steemit/chain/evaluators/account_witness_proxy_evaluator.hpp
        include/steemit/chain/evaluators/account_witness_vote_evaluator.hpp
        include/steemit/chain/evaluators/cancel_transfer_from_savings_evaluator.hpp
        include/steemit/chain/evaluators/challenge_authority_evaluator.hpp
        include/steemit/chain/evaluators/change_recovery_account_evaluator.hpp
        include/steemit/chain/evaluators/comment_evaluator.hpp
        include/steemit/chain/evaluators/comment_options_evaluator.hpp
        include/steemit/chain/evaluators/comment_payout_extension_evaluator.hpp
        include/steemit/chain/evaluators/convert_evaluator.hpp
        include/steemit/chain/evaluators/custom_binary_evaluator.hpp
        include/steemit/chain/evaluators/custom_evaluator.hpp
        include/steemit/chain/evaluators/custom_json_evaluator.hpp
        include/steemit/chain/evaluators/decline_voting_rights_evaluator.hpp
        include/steemit/chain/evaluators/delegate_vesting_shares_evaluator.hpp
        include/steemit/chain/evaluators/delete_comment_evaluator.hpp
        include/steemit/chain/evaluators/escrow_approve_evaluator.hpp
        include/steemit/chain/evaluators/escrow_dispute_evaluator.hpp
        include/steemit/chain/evaluators/escrow_release_evaluator.hpp
        include/steemit/chain/evaluators/escrow_transfer_evaluator.hpp
        include/steemit/chain/evaluators/feed_publish_evaluator.hpp
        include/steemit/chain/evaluators/forward.hpp
        include/steemit/chain/evaluators/limit_order_cancel_evaluator.hpp
        include/steemit/chain/evaluators/limit_order_create2_evaluator.hpp
        include/steemit/chain/evaluators/limit_order_create_evaluator.hpp
        include/steemit/chain/evaluators/pow2_evaluator.hpp
        include/steemit/chain/evaluators/pow_evaluator.hpp
        include/steemit/chain/evaluators/prove_authority_evaluator.hpp
        include/steemit/chain/evaluators/recover_account_evaluator.hpp
        include/steemit/chain/evaluators/report_over_production_evaluator.hpp
        include/steemit/chain/evaluators/request_account_recovery_evaluator.hpp
        include/steemit/chain/evaluators/reset_account_evaluator.hpp
        include/steemit/chain/evaluators/set_reset_account_evaluator.hpp
        include/steemit/chain/evaluators/set_withdraw_vesting_route_evaluator.hpp
        include/steemit/chain/evaluators/transfer_evaluator.hpp
        include/steemit/chain/evaluators/transfer_from_savings_evaluator.hpp
        include/steemit/chain/evaluators/transfer_to_savings_evaluator.hpp
        include/steemit/chain/evaluators/transfer_to_vesting_evaluator.hpp
        include/steemit/chain/evaluators/vote_evaluator.hpp
        include/steemit/chain/evaluators/withdraw_vesting_evaluator.hpp
        include/steemit/chain/evaluators/witness_update_evaluator.hpp

        include/steemit/chain/block_log.hpp
        include/steemit/chain/comment_object.hpp
        include/steemit/chain/compound.hpp
        include/steemit/chain/custom_operation_interpreter.hpp

        include/steemit/chain/db_with.hpp
        include/steemit/chain/fork_database.hpp
        include/steemit/chain/generic_custom_operation_interpreter.hpp
        include/steemit/chain/genesis_state.hpp
        include/steemit/chain/immutable_chain_parameters.hpp
        include/steemit/chain/index.hpp

        include/steemit/chain/chain_objects/node_property_object.hpp
        include/steemit/chain/operation_notification.hpp
        include/steemit/chain/shared_authority.hpp
        include/steemit/chain/shared_db_merkle.hpp

        include/steemit/chain/utilities/asset.hpp
        include/steemit/chain/utilities/reward.hpp
        include/steemit/chain/utilities/uint256.hpp

)

list(APPEND CURRENT_TARGET_SOURCES

        database/policies/account_policy.cpp
        database/policies/asset_policy.cpp
        database/policies/behaviour_based_policy.cpp
        database/policies/comment_policy.cpp
        database/policies/order_policy.cpp
        database/policies/reward_policy.cpp
        database/policies/withdrawal_policy.cpp
        database/policies/witness_schedule_policy.cpp
        database/policies/witness_policy.cpp
        database/policies/witness_schedule_policy.cpp

        database/worker/account_worker.cpp
        database/worker/asset_worker.cpp
        database/worker/behaviour_based_worker.cpp
        database/worker/reward_worker.cpp
        database/worker/witness_worker.cpp

        database/database.cpp
        database/database_basic.cpp
        database/generic_policy.cpp

        dynamic_extension/worker_storage.cpp
        dynamic_extension/smart_worker.cpp
        dynamic_extension/worker.cpp

        evaluators/account_create_evaluator.cpp
        evaluators/account_create_with_delegation_evaluator.cpp
        evaluators/account_update_evaluator.cpp
        evaluators/account_witness_proxy_evaluator.cpp
        evaluators/account_witness_vote_evaluator.cpp
        evaluators/cancel_transfer_from_savings_evaluator.cpp
        evaluators/challenge_authority_evaluator.cpp
        evaluators/change_recovery_account_evaluator.cpp
        evaluators/comment_evaluator.cpp
        evaluators/comment_options_evaluator.cpp
        evaluators/comment_payout_extension_evaluator.cpp
        evaluators/convert_evaluator.cpp
        evaluators/custom_binary_evaluator.cpp
        evaluators/custom_evaluator.cpp
        evaluators/custom_json_evaluator.cpp
        evaluators/decline_voting_rights_evaluator.cpp
        evaluators/delegate_vesting_shares_evaluator.cpp
        evaluators/delete_comment_evaluator.cpp
        evaluators/escrow_approve_evaluator.cpp
        evaluators/escrow_dispute_evaluator.cpp
        evaluators/escrow_release_evaluator.cpp
        evaluators/escrow_transfer_evaluator.cpp
        evaluators/feed_publish_evaluator.cpp
        evaluators/limit_order_cancel_evaluator.cpp
        evaluators/limit_order_create2_evaluator.cpp
        evaluators/limit_order_create_evaluator.cpp
        evaluators/pow2_evaluator.cpp
        evaluators/pow_evaluator.cpp
        evaluators/prove_authority_evaluator.cpp
        evaluators/recover_account_evaluator.cpp
        evaluators/report_over_production_evaluator.cpp
        evaluators/request_account_recovery_evaluator.cpp
        evaluators/reset_account_evaluator.cpp
        evaluators/set_reset_account_evaluator.cpp
        evaluators/set_withdraw_vesting_route_evaluator.cpp
        evaluators/transfer_evaluator.cpp
        evaluators/transfer_from_savings_evaluator.cpp
        evaluators/transfer_to_savings_evaluator.cpp
        evaluators/transfer_to_vesting_evaluator.cpp
        evaluators/vote_evaluator.cpp
        evaluators/withdraw_vesting_evaluator.cpp
        evaluators/witness_update_evaluator.cpp

        utilities/reward.cpp
        block_log.cpp
        fork_database.cpp
        forward.cpp
        shared_authority.cpp
        steem_objects.cpp
        #transaction_object.cpp
)

if(BUILD_SHARED_LIBRARIES)
    add_library(golos_${CURRENT_TARGET} SHARED
            ${CURRENT_TARGET_HEADERS}
            ${CURRENT_TARGET_SOURCES}

            ${hardfork_hpp_file}
            "${CMAKE_CURRENT_BINARY_DIR}/include/steemit/chain/hardfork.hpp"
            )
else()
    add_library(golos_${CURRENT_TARGET} STATIC
            ${CURRENT_TARGET_HEADERS}
            ${CURRENT_TARGET_SOURCES}

            ${hardfork_hpp_file}
            "${CMAKE_CURRENT_BINARY_DIR}/include/steemit/chain/hardfork.hpp"
            )
endif()

add_library(golos::${CURRENT_TARGET} ALIAS golos_${CURRENT_TARGET})
set_property(TARGET golos_${CURRENT_TARGET} PROPERTY EXPORT_NAME ${CURRENT_TARGET})

add_dependencies(golos_${CURRENT_TARGET} golos::protocol build_hardfork_hpp)

target_link_libraries(golos_${CURRENT_TARGET} golos::protocol fc chainbase graphene_schema ${PATCH_MERGE_LIB})
target_include_directories(golos_${CURRENT_TARGET}
        PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" "${CMAKE_CURRENT_BINARY_DIR}/include")

if(MSVC)
    set_source_files_properties(database/database_basic.cpp PROPERTIES COMPILE_FLAGS "/bigobj")
endif(MSVC)

install(TARGETS
        golos_${CURRENT_TARGET}

        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        )

install(FILES ${CURRENT_TARGET_HEADERS} DESTINATION "include/steemit/chain")
