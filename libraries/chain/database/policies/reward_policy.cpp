#include <steemit/chain/database/policies/reward_policy.hpp>
#include <steemit/chain/database/database_basic.hpp>
#include <steemit/chain/steem_objects.hpp>
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
                auto reward = references.get_liquidity_reward();

                if (reward.amount == 0) {
                    return;
                }

                const auto &ridx = references.get_index<liquidity_reward_balance_index>().indices().get<by_volume_weight>();
                auto itr = ridx.begin();
                if (itr != ridx.end() && itr->volume_weight() > 0) {
                    references.dynamic_extension_worker().get("account")->invoke("adjust_balance",reward, true);
                    references.dynamic_extension_worker().get("account")->invoke("adjust_balance",references.get(itr->owner), reward);
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
    }
}