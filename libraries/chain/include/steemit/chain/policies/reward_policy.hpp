#ifndef GOLOS_REWARD_POLICY_HPP
#define GOLOS_REWARD_POLICY_HPP

#include <steemit/chain/compound.hpp>

#include "generic_policy.hpp"

namespace steemit {
namespace chain {
struct reward_policy: public generic_policy {

    reward_policy() = default;

    reward_policy(const reward_policy &) = default;

    reward_policy &operator=(const reward_policy &) = default;

    reward_policy(reward_policy &&) = default;

    reward_policy &operator=(reward_policy &&) = default;

    virtual ~reward_policy() = default;

    reward_policy(database_basic &ref, evaluator_registry <operation> &evaluator_registry_) : generic_policy(ref){
    }

    const reward_fund_object &get_reward_fund(const comment_object &c) const {
        return get<reward_fund_object, by_name>(
                c.parent_author == STEEMIT_ROOT_POST_PARENT
                ? STEEMIT_POST_REWARD_FUND_NAME
                : STEEMIT_COMMENT_REWARD_FUND_NAME);
    }


    asset get_liquidity_reward() const {
        if(references.has_hardfork(STEEMIT_HARDFORK_0_12__178)) {
            return asset(0, STEEM_SYMBOL);
        }

        const auto &props = references.get_dynamic_global_properties();
        static_assert(STEEMIT_LIQUIDITY_REWARD_PERIOD_SEC == 60 * 60, "this code assumes a 1 hour time interval");
        asset percent(protocol::calc_percent_reward_per_hour<STEEMIT_LIQUIDITY_APR_PERCENT>(props.virtual_supply.amount), STEEM_SYMBOL);
        return std::max(percent, STEEMIT_MIN_LIQUIDITY_REWARD);
    }

    asset get_content_reward() const {
        const auto &props = references.get_dynamic_global_properties();
        auto reward = asset(255, STEEM_SYMBOL);
        static_assert(STEEMIT_BLOCK_INTERVAL ==
                      3, "this code assumes a 3-second time interval");
        if (props.head_block_number > STEEMIT_START_VESTING_BLOCK) {
            asset percent(protocol::calc_percent_reward_per_block<STEEMIT_CONTENT_APR_PERCENT>(props.virtual_supply.amount), STEEM_SYMBOL);
            reward = std::max(percent, STEEMIT_MIN_CONTENT_REWARD);
        }

        return reward;
    }

    asset get_curation_reward() const {
        const auto &props = references.get_dynamic_global_properties();
        auto reward = asset(85, STEEM_SYMBOL);
        static_assert(STEEMIT_BLOCK_INTERVAL == 3, "this code assumes a 3-second time interval");
        if (props.head_block_number > STEEMIT_START_VESTING_BLOCK) {
            asset percent(protocol::calc_percent_reward_per_block<STEEMIT_CURATE_APR_PERCENT>(props.virtual_supply.amount), STEEM_SYMBOL);
            reward = std::max(percent, STEEMIT_MIN_CURATE_REWARD);
        }

        return reward;
    }



    asset get_pow_reward() const {
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





    void pay_liquidity_reward() {
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
                adjust_supply(reward, true);
                adjust_balance(get(itr->owner), reward);
                references.modify(*itr, [&](liquidity_reward_balance_object &obj) {
                    obj.steem_volume = 0;
                    obj.sbd_volume = 0;
                    obj.last_update = references.head_block_time();
                    obj.weight = 0;
                });

                references.push_virtual_operation(liquidity_reward_operation(get(itr->owner).name, reward));
            }
        }
    }

    void retally_liquidity_weight() {
        const auto &ridx = references.get_index<liquidity_reward_balance_index>().indices().get<by_owner>();
        for (const auto &i : ridx) {
            references.modify(i, [](liquidity_reward_balance_object &o) {
                o.update_weight(true/*HAS HARDFORK10 if this method is called*/);
            });
        }
    }


    void adjust_liquidity_reward(const account_object &owner, const asset &volume, bool is_sdb) {
        const auto &ridx = references.get_index<liquidity_reward_balance_index>().indices().get<by_owner>();
        auto itr = ridx.find(owner.id);
        if (itr != ridx.end()) {
            references.modify<liquidity_reward_balance_object>(*itr, [&](liquidity_reward_balance_object &r) {
                if (references.head_block_time() - r.last_update >=
                    STEEMIT_LIQUIDITY_TIMEOUT_SEC) {
                    r.sbd_volume = 0;
                    r.steem_volume = 0;
                    r.weight = 0;
                }

                if (is_sdb) {
                    r.sbd_volume += volume.amount.value;
                } else {
                    r.steem_volume += volume.amount.value;
                }

                r.update_weight(references.has_hardfork(STEEMIT_HARDFORK_0_10__141));
                r.last_update = references.head_block_time();
            });
        } else {
            references.create<liquidity_reward_balance_object>([&](liquidity_reward_balance_object &r) {
                r.owner = owner.id;
                if (is_sdb) {
                    r.sbd_volume = volume.amount.value;
                } else {
                    r.steem_volume = volume.amount.value;
                }

                r.update_weight(references.has_hardfork(STEEMIT_HARDFORK_0_9__141));
                r.last_update = references.head_block_time();
            });
        }
    }


};
}}
#endif //GOLOS_REWARD_POLICY_HPP
