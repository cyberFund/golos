#include <steemit/protocol/chain_properties.hpp>

namespace steemit {
    namespace protocol {
        void chain_properties<1>::validate() const {
                FC_ASSERT(account_creation_fee.amount >= STEEMIT_MIN_ACCOUNT_CREATION_FEE);
                FC_ASSERT(maximum_block_size >= STEEMIT_MIN_BLOCK_SIZE_LIMIT);
                FC_ASSERT(sbd_interest_rate >= 0);
                FC_ASSERT(sbd_interest_rate <= STEEMIT_100_PERCENT);
        }

        void chain_properties<2>::validate() const {
            FC_ASSERT(block_interval > 0);
            FC_ASSERT(account_creation_fee.amount >=
                      STEEMIT_MIN_ACCOUNT_CREATION_FEE);
            FC_ASSERT(maximum_block_size >= STEEMIT_MIN_BLOCK_SIZE_LIMIT);
            FC_ASSERT(sbd_interest_rate >= 0);
            FC_ASSERT(sbd_interest_rate <= STEEMIT_100_PERCENT);
            FC_ASSERT(maximum_witness_count == maximum_votable_witness_count +
                                               maximum_miner_witness_count +
                                               maximum_runner_witness_count);
            FC_ASSERT(hardfork_required_witness_count <= maximum_witness_count);

            FC_ASSERT(inflation_content_reward_percent <= STEEMIT_100_PERCENT);
            FC_ASSERT(inflation_rate_stop_percent <= STEEMIT_100_PERCENT);
            FC_ASSERT(maximum_transaction_size >=
                      STEEMIT_MIN_TRANSACTION_SIZE_LIMIT,
                    "Transaction size limit is too low");
            FC_ASSERT(maximum_block_size >= STEEMIT_MIN_BLOCK_SIZE_LIMIT,
                    "Block size limit is too low");
            FC_ASSERT(maximum_time_until_expiration > block_interval,
                    "Maximum transaction expiration time must be greater than a block interval");
            FC_ASSERT(maximum_proposal_lifetime -
                      committee_proposal_review_period > block_interval,
                    "Committee proposal review period must be less than the maximum proposal lifetime");
        }
    }
}