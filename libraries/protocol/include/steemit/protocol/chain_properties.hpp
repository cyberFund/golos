#pragma once

#include <steemit/protocol/asset.hpp>
#include <steemit/protocol/base.hpp>
#include <steemit/protocol/types.hpp>

#include <fc/smart_ref_fwd.hpp>

namespace steemit {
    namespace protocol {
        typedef static_variant<> parameter_extension;

        /**
         * Witnesses must vote on how to set certain chain properties to ensure a smooth
         * and well functioning network.  Any time @owner is in the active set of witnesses these
         * properties will be used to control the blockchain configuration.
         */

        template<uint32_t VersionNumber>
        struct chain_properties : public static_version<VersionNumber> {};

        template<>
        struct chain_properties<1> {
        /**
             *  This fee, paid in STEEM, is converted into VESTING SHARES for the new account. Accounts
             *  without vesting shares cannot earn usage rations and therefore are powerless. This minimum
             *  fee requires all accounts to have some kind of commitment to the network that includes the
             *  ability to vote and make transactions.
             */
            asset account_creation_fee =
                    asset(STEEMIT_MIN_ACCOUNT_CREATION_FEE, STEEM_SYMBOL);

            /**
             *  This witnesses vote for the maximum_block_size which is used by the network
             *  to tune rate limiting and capacity
             */
            uint32_t maximum_block_size = STEEMIT_MIN_BLOCK_SIZE_LIMIT * 2;
            uint16_t sbd_interest_rate = STEEMIT_DEFAULT_SBD_INTEREST_RATE;

            void validate() const;
        };

        template<>
        struct chain_properties<2> {
            /**
             *  This fee, paid in STEEM, is converted into VESTING SHARES for the new account. Accounts
             *  without vesting shares cannot earn usage rations and therefore are powerless. This minimum
             *  fee requires all accounts to have some kind of commitment to the network that includes the
             *  ability to vote and make transactions.
             */
            asset account_creation_fee =
                    asset(STEEMIT_MIN_ACCOUNT_CREATION_FEE, STEEM_SYMBOL);

            /**
             *  This witnesses vote for the maximum_block_size which is used by the network
             *  to tune rate limiting and capacity
             */
            uint32_t maximum_block_size = STEEMIT_MIN_BLOCK_SIZE_LIMIT * 2;
            uint16_t sbd_interest_rate = STEEMIT_DEFAULT_SBD_INTEREST_RATE;

            uint32_t block_interval = STEEMIT_BLOCK_INTERVAL; ///< interval in seconds between blocks

            uint32_t cashout_window_seconds = STEEMIT_CASHOUT_WINDOW_SECONDS;
            uint32_t maximum_cashout_window_seconds = STEEMIT_MAX_CASHOUT_WINDOW_SECONDS;

            uint32_t vote_change_lockout_period = STEEMIT_VOTE_CHANGE_LOCKOUT_PERIOD;

            fc::microseconds owner_auth_recovery_period = STEEMIT_OWNER_AUTH_RECOVERY_PERIOD;
            fc::microseconds account_recovery_request_expiration_period = STEEMIT_ACCOUNT_RECOVERY_REQUEST_EXPIRATION_PERIOD;
            fc::microseconds owner_update_limit = STEEMIT_OWNER_UPDATE_LIMIT;

            uint32_t committee_proposal_review_period = STEEMIT_COMMITTEE_PROPOSAL_REVIEW_PERIOD_SEC; ///< minimum time in seconds that a proposed transaction requiring committee authority may not be signed, prior to expiration
            uint32_t maximum_transaction_size = STEEMIT_MAX_TRANSACTION_SIZE; ///< maximum allowable size in bytes for a transaction
            uint32_t maximum_time_until_expiration = STEEMIT_MAX_TIME_UNTIL_EXPIRATION; ///< maximum lifetime in seconds for transactions to be valid, before expiring
            uint32_t maximum_proposal_lifetime = STEEMIT_MAX_PROPOSAL_LIFETIME_SEC; ///< maximum lifetime in seconds for proposed transactions to be kept, before expiring
            uint32_t maximum_witness_count = STEEMIT_MAX_WITNESSES; ///< maximum number of active witnesses
            uint32_t maximum_votable_witness_count = STEEMIT_MAX_VOTED_WITNESSES;
            uint32_t maximum_miner_witness_count = STEEMIT_MAX_MINER_WITNESSES;
            uint32_t maximum_runner_witness_count = STEEMIT_MAX_RUNNER_WITNESSES;

            uint32_t hardfork_required_witness_count = STEEMIT_HARDFORK_REQUIRED_WITNESSES;

            uint32_t maximum_memo_size = STEEMIT_MAX_MEMO_SIZE;

            uint32_t vesting_withdraw_intervals = STEEMIT_VESTING_WITHDRAW_INTERVALS;
            uint32_t vesting_withdraw_interval_seconds = STEEMIT_VESTING_WITHDRAW_INTERVAL_SECONDS;

            fc::microseconds savings_withdraw_time = STEEMIT_SAVINGS_WITHDRAW_TIME;
            uint32_t savings_withdraw_request_limit = STEEMIT_SAVINGS_WITHDRAW_REQUEST_LIMIT;

            uint32_t vote_regeneration_seconds = STEEMIT_VOTE_REGENERATION_SECONDS;

            uint32_t maximum_vote_changes = STEEMIT_MAX_VOTE_CHANGES;

            fc::microseconds upvote_lockout = STEEMIT_UPVOTE_LOCKOUT;

            uint32_t minimum_vote_interval_seconds = STEEMIT_MIN_VOTE_INTERVAL_SEC;

            fc::microseconds minimum_root_comment_interval = STEEMIT_MIN_ROOT_COMMENT_INTERVAL;
            fc::microseconds minimum_reply_interval = STEEMIT_MIN_REPLY_INTERVAL;

            uint32_t maximum_post_bandwidth = STEEMIT_POST_MAX_BANDWIDTH;

            uint32_t maximum_account_witness_votes = STEEMIT_MAX_ACCOUNT_WITNESS_VOTES;

            uint32_t inflation_rate_start_percent = STEEMIT_INFLATION_RATE_START_PERCENT;
            uint32_t inflation_rate_stop_percent = STEEMIT_INFLATION_RATE_STOP_PERCENT;
            uint32_t inflation_narrowing_period = STEEMIT_INFLATION_NARROWING_PERIOD;
            double inflation_content_reward_percent = STEEMIT_INFLATION_CONTENT_REWARD_PERCENT;
            double inflation_vesting_fund_reward_percent = STEEMIT_INFLATION_VESTING_FUND_PERCENT;

            uint32_t miner_pay_percent = STEEMIT_MINER_PAY_PERCENT;
            asset miner_reward = STEEMIT_MINING_REWARD;

            uint32_t payout_extension_cost_per_day = STEEMIT_PAYOUT_EXTENSION_COST_PER_DAY;

            uint32_t post_reward_fund_percent = STEEMIT_POST_REWARD_FUND_PERCENT;
            uint32_t comment_reward_fund_percent = STEEMIT_COMMENT_REWARD_FUND_PERCENT;

            uint32_t maximum_committee_count = STEEMIT_MAX_COMMITTEE; ///< maximum number of active committee_members

//            uint32_t maximum_authority_membership = STEEMIT_MAX_AUTHORITY_MEMBERSHIP; ///< largest number of keys/accounts an authority can have
//            uint32_t cashback_vesting_period_seconds = STEEMIT_CASHBACK_VESTING_PERIOD_SEC; ///< time after cashback rewards are accrued before they become liquid
//            share_type cashback_vesting_threshold = STEEMIT_CASHBACK_VESTING_THRESHOLD; ///< the maximum cashback that can be received without vesting
            bool count_non_member_votes = true; ///< set to false to restrict voting privlegages to member accounts
            bool allow_non_member_whitelists = false; ///< true if non-member accounts may set whitelists and blacklists; false otherwise

            uint32_t max_authority_depth = STEEMIT_MAX_SIG_CHECK_DEPTH;

            extensions_type extensions;

            void validate() const;
        };

    }
}  // steemit::protocol

FC_REFLECT(steemit::protocol::chain_properties<1>, (account_creation_fee)(maximum_block_size)(sbd_interest_rate));

FC_REFLECT(steemit::protocol::chain_properties<2>,
        (account_creation_fee)
                (maximum_block_size)
                (sbd_interest_rate)
                (block_interval)
                (cashout_window_seconds)
                (maximum_cashout_window_seconds)
                (vote_change_lockout_period)
                (owner_auth_recovery_period)
                (account_recovery_request_expiration_period)
                (owner_update_limit)

                (committee_proposal_review_period)
                (maximum_transaction_size)
                (maximum_time_until_expiration)
                (maximum_proposal_lifetime)

                (maximum_witness_count)
                (maximum_votable_witness_count)
                (maximum_miner_witness_count)
                (maximum_runner_witness_count)

                (hardfork_required_witness_count)

                (maximum_memo_size)

                (vesting_withdraw_intervals)
                (vesting_withdraw_interval_seconds)

                (savings_withdraw_time)
                (savings_withdraw_request_limit)

                (vote_regeneration_seconds)

                (maximum_vote_changes)

                (upvote_lockout)

                (minimum_vote_interval_seconds)

                (minimum_root_comment_interval)
                (minimum_reply_interval)

                (maximum_post_bandwidth)

                (maximum_account_witness_votes)

                (inflation_rate_start_percent)
                (inflation_rate_stop_percent)
                (inflation_narrowing_period)
                (inflation_content_reward_percent)
                (inflation_vesting_fund_reward_percent)

                (miner_pay_percent)
                (miner_reward)

                (payout_extension_cost_per_day)

                (post_reward_fund_percent)
                (comment_reward_fund_percent)

                (maximum_committee_count)

                (count_non_member_votes)
                (allow_non_member_whitelists)

                (max_authority_depth)
                (extensions)
)