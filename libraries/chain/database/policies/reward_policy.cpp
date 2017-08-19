#include <steemit/chain/database/policies/reward_policy.hpp>
#include <steemit/chain/database/database_basic.hpp>
#include <steemit/chain/chain_objects/steem_objects.hpp>
#include <steemit/chain/compound.hpp>
#include <steemit/chain/database/big_helper.hpp>

namespace steemit {
    namespace chain {

        reward_policy::reward_policy(database_basic &ref,int) : generic_policy(ref){
        }

        steemit::protocol::asset reward_policy::get_pow_reward() const {
            const auto &props = references.get_dynamic_global_properties();

#ifndef STEEMIT_BUILD_TESTNET
            /// 0 block rewards until at least STEEMIT_MAX_WITNESSES have produced a POW
            if (props.num_pow_witnesses < STEEMIT_MAX_WITNESSES &&
                props.head_block_number < STEEMIT_START_VESTING_BLOCK) {
                return asset(0, STEEM_SYMBOL);
            }
#endif

            static_assert(STEEMIT_BLOCK_INTERVAL == 3, "this code assumes a 3-second time interval");
//            static_assert(STEEMIT_MAX_WITNESSES ==
//                          21, "this code assumes 21 per round");
            asset percent(calc_percent_reward_per_round<STEEMIT_POW_APR_PERCENT>(props.virtual_supply.amount), STEEM_SYMBOL);

            if (references.has_hardfork(STEEMIT_HARDFORK_0_16)) {
                return std::max(percent, STEEMIT_MIN_POW_REWARD);
            } else {
                return std::max(percent, STEEMIT_MIN_POW_REWARD_PRE_HF16);
            }
        }

        void reward_policy::pay_liquidity_reward() {
#ifdef STEEMIT_BUILD_TESTNET
            if (!liquidity_rewards_enabled) {
                return;
            }
#endif

            if ((references.head_block_num() % STEEMIT_LIQUIDITY_REWARD_BLOCKS) == 0) {
                auto reward = get_liquidity_reward();

                if (reward.amount == 0) {
                    return;
                }

                const auto &ridx = references.get_index<liquidity_reward_balance_index>().indices().get<by_volume_weight>();
                auto itr = ridx.begin();
                if (itr != ridx.end() && itr->volume_weight() > 0) {
                    database_helper::big_helper::adjust_supply(references,reward, true);
                    database_helper::big_helper::adjust_balance(references,references.get(itr->owner), reward);
                    references.modify(*itr, [&](liquidity_reward_balance_object &obj) {
                        obj.steem_volume = 0;
                        obj.sbd_volume = 0;
                        obj.last_update = references.head_block_time();
                        obj.weight = 0;
                    });

                    references.push_virtual_operation(liquidity_reward_operation(references.get(itr->owner).name, reward));
                }
            }
        }

        void reward_policy::retally_liquidity_weight() {
            const auto &ridx = references.get_index<liquidity_reward_balance_index>().indices().get<by_owner>();
            for (const auto &i : ridx) {
                references.modify(i, [](liquidity_reward_balance_object &o) {
                    o.update_weight(true/*HAS HARDFORK10 if this method is called*/);
                });
            }
        }

        asset reward_policy::get_liquidity_reward() const {

            if(references.has_hardfork(STEEMIT_HARDFORK_0_12__178)) {
                return asset(0, STEEM_SYMBOL);
            }

            const auto &props = references.get_dynamic_global_properties();
            static_assert(STEEMIT_LIQUIDITY_REWARD_PERIOD_SEC == 60 * 60, "this code assumes a 1 hour time interval");
            asset percent(protocol::calc_percent_reward_per_hour<STEEMIT_LIQUIDITY_APR_PERCENT>(props.virtual_supply.amount), STEEM_SYMBOL);
            return std::max(percent, STEEMIT_MIN_LIQUIDITY_REWARD);
        }

        asset reward_policy::get_content_reward() const {
            const auto &props = references.get_dynamic_global_properties();
            auto reward = asset(255, STEEM_SYMBOL);
            static_assert(STEEMIT_BLOCK_INTERVAL == 3, "this code assumes a 3-second time interval");
            if (props.head_block_number > STEEMIT_START_VESTING_BLOCK) {
                asset percent(protocol::calc_percent_reward_per_block<STEEMIT_CONTENT_APR_PERCENT>(props.virtual_supply.amount), STEEM_SYMBOL);
                reward = std::max(percent, STEEMIT_MIN_CONTENT_REWARD);
            }

            return reward;
        }

        asset reward_policy::get_curation_reward() const {
            const auto &props = references.get_dynamic_global_properties();
            auto reward = asset(85, STEEM_SYMBOL);
            static_assert(STEEMIT_BLOCK_INTERVAL == 3, "this code assumes a 3-second time interval");
            if (props.head_block_number > STEEMIT_START_VESTING_BLOCK) {
                asset percent(protocol::calc_percent_reward_per_block<STEEMIT_CURATE_APR_PERCENT>(props.virtual_supply.amount), STEEM_SYMBOL);
                reward = std::max(percent, STEEMIT_MIN_CURATE_REWARD);
            }

            return reward;
        }

        const reward_fund_object &reward_policy::get_reward_fund(const comment_object &c) const {
            return references.get<reward_fund_object, by_name>(c.parent_author == STEEMIT_ROOT_POST_PARENT ? STEEMIT_POST_REWARD_FUND_NAME : STEEMIT_COMMENT_REWARD_FUND_NAME);
        }
    }
}