#pragma once

#include <steemit/protocol/operations/operation_utilities.hpp>
#include <steemit/protocol/operations/asset_operations.hpp>
#include <steemit/protocol/operations/market_operations.hpp>
#include <steemit/protocol/operations/market_virtual_operations.hpp>
#include <steemit/protocol/operations/steem_operations.hpp>
#include <steemit/protocol/operations/steem_virtual_operations.hpp>
#include <steemit/protocol/operations/asset_virtual_operations.hpp>
#include <steemit/protocol/operations/account_operations.hpp>
#include <steemit/protocol/operations/transfer_operations.hpp>
#include <steemit/protocol/operations/proposal_operations.hpp>

namespace steemit {
    namespace protocol {

        /** NOTE: do not change the order of any operations prior to the virtual operations
         * or it will trigger a hardfork.
         */
        typedef fc::static_variant<
                vote_operation,
                comment_operation,

                transfer_operation,
                transfer_to_vesting_operation,
                withdraw_vesting_operation,

                limit_order_create_operation,
                limit_order_cancel_operation,

                feed_publish_operation,
                convert_operation,

                account_create_operation,
                account_update_operation,

                witness_update_operation,
                account_witness_vote_operation,
                account_witness_proxy_operation,

                pow_operation,

                custom_operation,

                report_over_production_operation,

                delete_comment_operation,
                custom_json_operation,
                comment_options_operation,
                set_withdraw_vesting_route_operation,
                limit_order_create2_operation,
                challenge_authority_operation,
                prove_authority_operation,
                request_account_recovery_operation,
                recover_account_operation,
                change_recovery_account_operation,

                escrow_transfer_operation,
                escrow_dispute_operation,
                escrow_release_operation,

                pow2_operation,
                escrow_approve_operation,

                transfer_to_savings_operation,
                transfer_from_savings_operation,
                cancel_transfer_from_savings_operation,

                custom_binary_operation,
                decline_voting_rights_operation,
                reset_account_operation,
                set_reset_account_operation,
                comment_benefactor_reward_operation,
                delegate_vesting_shares_operation,
                account_create_with_delegation_operation,
                comment_payout_extension_operation,

                asset_create_operation,
                asset_update_operation,
                asset_update_bitasset_operation,
                asset_update_feed_producers_operation,
                asset_issue_operation,
                asset_reserve_operation,
                asset_fund_fee_pool_operation,
                asset_settle_operation,
                asset_force_settle_operation,
                asset_global_settle_operation,
                asset_publish_feed_operation,
                asset_claim_fees_operation,

                call_order_update_operation,

                account_whitelist_operation,

                override_transfer_operation,

                proposal_create_operation,
                proposal_update_operation,
                proposal_delete_operation,

                bid_collateral_operation,

                /// virtual operations below this point
                fill_convert_request_operation,
                author_reward_operation,
                curation_reward_operation,
                comment_reward_operation,
                liquidity_reward_operation,
                interest_operation,
                fill_vesting_withdraw_operation,
                fill_order_operation,
                shutdown_witness_operation,
                fill_transfer_from_savings_operation,
                hardfork_operation,
                comment_payout_update_operation,
                return_vesting_delegation_operation,
                asset_settle_cancel_operation,
                fill_call_order_operation,
                fill_settlement_order_operation,
                execute_bid_operation
        > operation;

        /*void operation_get_required_authorities( const operation& op,
                                                 flat_set<string>& active,
                                                 flat_set<string>& owner,
                                                 flat_set<string>& posting,
                                                 vector<authority>&  other );

        void operation_validate( const operation& op );*/

        bool is_market_operation(const operation &op);

        bool is_virtual_operation(const operation &op);

        struct operation_wrapper {
        public:
            operation_wrapper(const operation &op = operation()) : op(op) {
            }

            operation op;
        };
    }
} // steemit::protocol

/*namespace fc {
    void to_variant( const steemit::protocol::operation& var,  fc::variant& vo );
    void from_variant( const fc::variant& var,  steemit::protocol::operation& vo );
}*/

STEEMIT_DECLARE_OPERATION_TYPE(steemit::protocol::operation);
FC_REFLECT_TYPENAME(steemit::protocol::operation);

FC_REFLECT(steemit::protocol::operation_wrapper, (op));
