#ifndef GOLOS_CHAIN_PROPERTIES_HPP
#define GOLOS_CHAIN_PROPERTIES_HPP

#include <golos/protocol/asset.hpp>
#include <golos/protocol/base.hpp>

namespace golos {
    namespace protocol {
        /**
         * Witnesses must vote on how to set certain chain properties to ensure a smooth
         * and well functioning network.  Any time @owner is in the active set of witnesses these
         * properties will be used to control the blockchain configuration.
         */
        template<uint8_t Major, uint8_t Hardfork, uint16_t Release, typename = type_traits::static_range<true>>
        struct chain_properties {

        };

        template<uint8_t Major, uint8_t Hardfork, uint16_t Release>
        struct chain_properties<Major, Hardfork, Release, type_traits::static_range<Hardfork <= 16>> {
            /**
             *  This fee, paid in STEEM, is converted into VESTING SHARES for the new account. Accounts
             *  without vesting shares cannot earn usage rations and therefore are powerless. This minimum
             *  fee requires all accounts to have some kind of commitment to the network that includes the
             *  ability to vote and make transactions.
             */
            asset <Major, Hardfork, Release> account_creation_fee = {STEEMIT_MIN_ACCOUNT_CREATION_FEE, STEEM_SYMBOL_NAME};

            /**
             *  This witnesses vote for the maximum_block_size which is used by the network
             *  to tune rate limiting and capacity
             */
            uint32_t maximum_block_size = STEEMIT_MIN_BLOCK_SIZE_LIMIT * 2;
            uint16_t sbd_interest_rate = STEEMIT_DEFAULT_SBD_INTEREST_RATE;

            void validate() const {
                FC_ASSERT(account_creation_fee.amount >= STEEMIT_MIN_ACCOUNT_CREATION_FEE);
                FC_ASSERT(maximum_block_size >= STEEMIT_MIN_BLOCK_SIZE_LIMIT);
                FC_ASSERT(sbd_interest_rate >= 0);
                FC_ASSERT(sbd_interest_rate <= STEEMIT_100_PERCENT);
            }
        };

        template<uint8_t Major, uint8_t Hardfork, uint16_t Release>
        struct chain_properties<Major, Hardfork, Release, type_traits::static_range<Hardfork >= 17>> {
            /**
             *  This fee, paid in GOLOS, is converted into VESTING SHARES for the new account. Accounts
             *  without vesting shares cannot earn usage rations and therefore are powerless. This minimum
             *  fee requires all accounts to have some kind of commitment to the network that includes the
             *  ability to vote and make transactions.
             */
            asset <Major, Hardfork, Release> account_creation_fee = {STEEMIT_MIN_ACCOUNT_CREATION_FEE, STEEM_SYMBOL_NAME};

            /**
             *  This fee, paid in GBG, is converted into VESTING SHARES for the new account. Accounts
             *  without vesting shares cannot earn usage rations and therefore are powerless. This minimum
             *  fee requires all accounts to have some kind of commitment to the network that includes the
             *  ability to vote and make transactions.
             */
            asset <Major, Hardfork, Release> asset_creation_fee = {STEEMIT_MIN_ASSET_CREATION_FEE, SBD_SYMBOL_NAME};

            /**
             *  This witnesses vote for the maximum_block_size which is used by the network
             *  to tune rate limiting and capacity
             */
            uint32_t maximum_block_size = STEEMIT_MIN_BLOCK_SIZE_LIMIT * 2;
            uint16_t sbd_interest_rate = STEEMIT_DEFAULT_SBD_INTEREST_RATE;

            uint32_t producer_duration_name_cost = STEEMIT_DEFAULT_PRODUCER_REWARD_DURATION;

            void validate() const {
                FC_ASSERT(account_creation_fee.amount >= STEEMIT_MIN_ACCOUNT_CREATION_FEE);
                FC_ASSERT(asset_creation_fee.amount >= STEEMIT_MIN_ASSET_CREATION_FEE);
                FC_ASSERT(maximum_block_size >= STEEMIT_MIN_BLOCK_SIZE_LIMIT);
                FC_ASSERT(sbd_interest_rate >= 0);
                FC_ASSERT(sbd_interest_rate <= STEEMIT_100_PERCENT);
            }
        };
    }
}

FC_REFLECT((golos::protocol::chain_properties<0, 16, 0>), (account_creation_fee)(maximum_block_size)(sbd_interest_rate));
FC_REFLECT((golos::protocol::chain_properties<0, 17, 0>), (account_creation_fee)(asset_creation_fee)(maximum_block_size)(sbd_interest_rate)(producer_duration_name_cost));

#endif //GOLOS_CHAIN_PROPERTIES_HPP