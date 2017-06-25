#pragma once

#include <fc/uint128.hpp>

#include <steemit/chain/steem_object_types.hpp>

#include <steemit/protocol/asset.hpp>
#include <steemit/protocol/chain_properties.hpp>
#include <steemit/protocol/vote.hpp>

namespace steemit {
    namespace chain {

        using steemit::protocol::asset;
        using steemit::protocol::price;

        /**
         * @class global_property_object
         * @brief Maintains global state information (committee_member list, current fees)
         * @ingroup object
         * @ingroup implementation
         *
         * This is an implementation detail. The values here are set by committee_members to tune the blockchain parameters.
         */
        template<protocol::version_type VersionNumber>
        class global_property_object
                : public object<global_property_object_type, global_property_object<VersionNumber>, VersionNumber>,
                  public protocol::static_version<VersionNumber> {
        };

        template<>
        class global_property_object<1>
                : public object<global_property_object_type, global_property_object<1>, 1> {
        public:
            template<typename Constructor, typename Allocator>
            global_property_object(Constructor &&c, allocator<Allocator> a) {
                c(*this);
            }

            global_property_object() {

            }

            protocol::vote_id_type get_next_vote_id(protocol::vote_id_type::vote_type type) {
                return protocol::vote_id_type(type, next_available_vote_id++);
            }

            id_type id;

            protocol::chain_properties<2> parameters;
            optional<protocol::chain_properties<2>> pending_parameters;

            uint32_t next_available_vote_id = 0;
            std::vector<committee_member_id_type> active_committee_members; // updated once per maintenance interval
            flat_set<witness_id_type> active_witnesses; // updated once per maintenance interval
            // n.b. witness scheduling is done by witness_schedule object
        };

        typedef multi_index_container<
                global_property_object<1>,
                indexed_by<
                        ordered_unique<tag<by_id>,
                                member<global_property_object<1>, global_property_object<1>::id_type, &global_property_object<1>::id>>
                >,
                allocator<global_property_object<1>>
        > global_property_index;

        /**
         * @class dynamic_global_property_object
         * @brief Maintains global state information
         * @ingroup object
         * @ingroup implementation
         *
         * This is an implementation detail. The values here are calculated during normal chain operations and reflect the current values of global blockchain properties.
         */

        template<protocol::version_type VersionNumber>
        class dynamic_global_property_object
                : public object<dynamic_global_property_object_type, dynamic_global_property_object<VersionNumber>, VersionNumber>,
                  public protocol::static_version<VersionNumber> {
        };

        template<>
        class dynamic_global_property_object<1>
                : public object<dynamic_global_property_object_type, dynamic_global_property_object<1>, 1>,
                  public protocol::static_version<1> {
        public:
            template<typename Constructor, typename Allocator>
            dynamic_global_property_object(Constructor &&c, allocator<Allocator> a) {
                c(*this);
            }

            dynamic_global_property_object() {

            }

            id_type id;

            uint32_t head_block_number = 0;
            block_id_type head_block_id;
            time_point_sec time;
            account_name_type current_witness;


            /**
             *  The total POW accumulated, aka the sum of num_pow_witness at the time new POW is added
             */
            uint64_t total_pow = -1;

            /**
             * The current count of how many pending POW witnesses there are, determines the difficulty
             * of doing pow
             */
            uint32_t num_pow_witnesses = 0;

            asset virtual_supply = asset(0, STEEM_SYMBOL);
            asset current_supply = asset(0, STEEM_SYMBOL);
            asset confidential_supply = asset(0, STEEM_SYMBOL); ///< total asset held in confidential balances
            asset current_sbd_supply = asset(0, SBD_SYMBOL);
            asset confidential_sbd_supply = asset(0, SBD_SYMBOL); ///< total asset held in confidential balances
            asset total_vesting_fund_steem = asset(0, STEEM_SYMBOL);
            asset total_vesting_shares = asset(0, VESTS_SYMBOL);
            asset total_reward_fund_steem = asset(0, STEEM_SYMBOL);
            fc::uint128 total_reward_shares2; ///< the running total of REWARD^2

            price get_vesting_share_price() const {
                if (total_vesting_fund_steem.amount == 0 ||
                    total_vesting_shares.amount == 0) {
                    return price(asset(1000, STEEM_SYMBOL), asset(1000000, VESTS_SYMBOL));
                }

                return price(total_vesting_shares, total_vesting_fund_steem);
            }

            /**
             *  This property defines the interest rate that SBD deposits receive.
             */
            uint16_t sbd_interest_rate = 0;

            uint16_t sbd_print_rate = STEEMIT_100_PERCENT;

            /**
             *  Average block size is updated every block to be:
             *
             *     average_block_size = (99 * average_block_size + new_block_size) / 100
             *
             *  This property is used to update the current_reserve_ratio to maintain approximately
             *  50% or less utilization of network capacity.
             */
            uint32_t average_block_size = 0;

            /**
             *  Maximum block size is decided by the set of active witnesses which change every round.
             *  Each witness posts what they think the maximum size should be as part of their witness
             *  properties, the median size is chosen to be the maximum block size for the round.
             *
             *  @note the minimum value for maximum_block_size is defined by the protocol to prevent the
             *  network from getting stuck by witnesses attempting to set this too low.
             */
            uint32_t maximum_block_size = 0;

            /**
             * The current absolute slot number.  Equal to the total
             * number of slots since genesis.  Also equal to the total
             * number of missed slots plus head_block_number.
             */
            uint64_t current_aslot = 0;

            /**
             * used to compute witness participation.
             */
            fc::uint128_t recent_slots_filled;
            uint8_t participation_count = 0; ///< Divide by 128 to compute participation percentage

            uint32_t last_irreversible_block_num = 0;

            /**
             * The maximum bandwidth the blockchain can support is:
             *
             *    max_bandwidth = maximum_block_size * STEEMIT_BANDWIDTH_AVERAGE_WINDOW_SECONDS / STEEMIT_BLOCK_INTERVAL
             *
             * The maximum virtual bandwidth is:
             *
             *    max_bandwidth * current_reserve_ratio
             */
            uint64_t max_virtual_bandwidth = 0;

            /**
             *   Any time average_block_size <= 50% maximum_block_size this value grows by 1 until it
             *   reaches STEEMIT_MAX_RESERVE_RATIO.  Any time average_block_size is greater than
             *   50% it falls by 1%.  Upward adjustments happen once per round, downward adjustments
             *   happen every block.
             */
            uint64_t current_reserve_ratio = 1;

            /**
             * The number of votes regenerated per day.  Any user voting slower than this rate will be
             * "wasting" voting power through spillover; any user voting faster than this rate will have
             * their votes reduced.
             */
            uint32_t vote_regeneration_per_day = 40;
        };

        typedef multi_index_container<
                dynamic_global_property_object<1>,
                indexed_by<
                        ordered_unique<tag<by_id>,
                                member<dynamic_global_property_object<1>, dynamic_global_property_object<1>::id_type, &dynamic_global_property_object<1>::id>>
                >,
                allocator<dynamic_global_property_object<1>>
        > dynamic_global_property_index;

    }
} // steemit::chain

FC_REFLECT(steemit::chain::dynamic_global_property_object<1>,
        (id)
                (head_block_number)
                (head_block_id)
                (time)
                (current_witness)
                (total_pow)
                (num_pow_witnesses)
                (virtual_supply)
                (current_supply)
                (confidential_supply)
                (current_sbd_supply)
                (confidential_sbd_supply)
                (total_vesting_fund_steem)
                (total_vesting_shares)
                (total_reward_fund_steem)
                (total_reward_shares2)
                (sbd_interest_rate)
                (sbd_print_rate)
                (average_block_size)
                (maximum_block_size)
                (current_aslot)
                (recent_slots_filled)
                (participation_count)
                (last_irreversible_block_num)
                (max_virtual_bandwidth)
                (current_reserve_ratio)
                (vote_regeneration_per_day)
)
CHAINBASE_SET_INDEX_TYPE(steemit::chain::dynamic_global_property_object<1>,
        steemit::chain::dynamic_global_property_index)

FC_REFLECT(steemit::chain::global_property_object<1>,
        (parameters)
                (pending_parameters)
                (next_available_vote_id)
                (active_committee_members)
                (active_witnesses)
)
CHAINBASE_SET_INDEX_TYPE(steemit::chain::global_property_object<1>, steemit::chain::global_property_index)