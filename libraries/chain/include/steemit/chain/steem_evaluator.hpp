#pragma once

#include <steemit/protocol/steem_operations.hpp>

#include <steemit/chain/evaluator.hpp>

namespace steemit {
    namespace chain {

        using namespace steemit::protocol;

        STEEMIT_DEFINE_EVALUATOR(account_create)

        STEEMIT_DEFINE_EVALUATOR(account_create_with_delegation)

        STEEMIT_DEFINE_EVALUATOR(account_update)

        STEEMIT_DEFINE_EVALUATOR(transfer)

        STEEMIT_DEFINE_EVALUATOR(transfer_to_vesting)

        STEEMIT_DEFINE_EVALUATOR(witness_update)

        STEEMIT_DEFINE_EVALUATOR(account_witness_vote)

        STEEMIT_DEFINE_EVALUATOR(account_witness_proxy)

        STEEMIT_DEFINE_EVALUATOR(withdraw_vesting)

        STEEMIT_DEFINE_EVALUATOR(set_withdraw_vesting_route)

        STEEMIT_DEFINE_EVALUATOR(comment)

        STEEMIT_DEFINE_EVALUATOR(comment_options)

        STEEMIT_DEFINE_EVALUATOR(comment_payout_extension)

        STEEMIT_DEFINE_EVALUATOR(delete_comment)

        STEEMIT_DEFINE_EVALUATOR(vote)

        STEEMIT_DEFINE_EVALUATOR(custom)

        STEEMIT_DEFINE_EVALUATOR(custom_json)

        STEEMIT_DEFINE_EVALUATOR(custom_binary)

        STEEMIT_DEFINE_EVALUATOR(pow)

        STEEMIT_DEFINE_EVALUATOR(pow2)

        STEEMIT_DEFINE_EVALUATOR(feed_publish)

        STEEMIT_DEFINE_EVALUATOR(convert)

        STEEMIT_DEFINE_EVALUATOR(limit_order_create)

        STEEMIT_DEFINE_EVALUATOR(limit_order_cancel)

        STEEMIT_DEFINE_EVALUATOR(report_over_production)

        STEEMIT_DEFINE_EVALUATOR(limit_order_create2)

        STEEMIT_DEFINE_EVALUATOR(escrow_transfer)

        STEEMIT_DEFINE_EVALUATOR(escrow_approve)

        STEEMIT_DEFINE_EVALUATOR(escrow_dispute)

        STEEMIT_DEFINE_EVALUATOR(escrow_release)

        STEEMIT_DEFINE_EVALUATOR(challenge_authority)

        STEEMIT_DEFINE_EVALUATOR(prove_authority)

        STEEMIT_DEFINE_EVALUATOR(request_account_recovery)

        STEEMIT_DEFINE_EVALUATOR(recover_account)

        STEEMIT_DEFINE_EVALUATOR(change_recovery_account)

        STEEMIT_DEFINE_EVALUATOR(transfer_to_savings)

        STEEMIT_DEFINE_EVALUATOR(transfer_from_savings)

        STEEMIT_DEFINE_EVALUATOR(cancel_transfer_from_savings)

        STEEMIT_DEFINE_EVALUATOR(decline_voting_rights)

        STEEMIT_DEFINE_EVALUATOR(reset_account)

        STEEMIT_DEFINE_EVALUATOR(set_reset_account)

        STEEMIT_DEFINE_EVALUATOR(delegate_vesting_shares)
    }
} // steemit::chain
