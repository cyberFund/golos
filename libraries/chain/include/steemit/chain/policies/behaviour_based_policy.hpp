#ifndef GOLOS_BEHAVIOUR_BASED_POLICY_HPP
#define GOLOS_BEHAVIOUR_BASED_POLICY_HPP

#include "generic_policy.hpp"

namespace steemit {
namespace chain {

struct behaviour_based_policy: public generic_policy {
public:
    behaviour_based_policy() = default;

    behaviour_based_policy(const behaviour_based_policy &) = default;

    behaviour_based_policy &operator=(const behaviour_based_policy &) = default;

    behaviour_based_policy(behaviour_based_policy &&) = default;

    behaviour_based_policy &operator=(behaviour_based_policy &&) = default;

    virtual ~behaviour_based_policy() = default;

    behaviour_based_policy(database_basic &ref, evaluator_registry <operation> &evaluator_registry_) : generic_policy(ref) {
    }

    share_type pay_reward_funds(share_type reward) {
        const auto &reward_idx = references.get_index<reward_fund_index, by_id>();
        share_type used_rewards = 0;

        for (auto itr = reward_idx.begin();
             itr != reward_idx.end(); ++itr) {
            // reward is a per block reward and the percents are 16-bit. This should never overflow
            auto r = (reward * itr->percent_content_rewards) / STEEMIT_100_PERCENT;

            references.modify(*itr, [&](reward_fund_object &rfo) {
                rfo.reward_balance += asset(r, STEEM_SYMBOL);
            });

            used_rewards += r;

            FC_ASSERT(used_rewards <= reward);
        }

        return used_rewards;
    }

    /**
 *  Converts STEEM into sbd and adds it to to_account while reducing the STEEM supply
 *  by STEEM and increasing the sbd supply by the specified amount.
 */
    std::pair<asset, asset> create_sbd(const account_object &to_account, asset steem) {
        std::pair<asset, asset> assets(asset(0, SBD_SYMBOL), asset(0, STEEM_SYMBOL));

        try {
            if (steem.amount == 0) {
                return assets;
            }

            const auto &median_price = references.get_feed_history().current_median_history;
            const auto &gpo = references.get_dynamic_global_properties();

            if (!median_price.is_null()) {
                auto to_sbd = (gpo.sbd_print_rate * steem.amount) /
                              STEEMIT_100_PERCENT;
                auto to_steem = steem.amount - to_sbd;

                auto sbd = asset(to_sbd, STEEM_SYMBOL) * median_price;

                adjust_balance(to_account, sbd);
                adjust_balance(to_account, asset(to_steem, STEEM_SYMBOL));
                adjust_supply(asset(-to_sbd, STEEM_SYMBOL));
                adjust_supply(sbd);
                assets.first = sbd;
                assets.second = to_steem;
            } else {
                adjust_balance(to_account, steem);
                assets.second = steem;
            }
        }
        FC_CAPTURE_LOG_AND_RETHROW((to_account.name)(steem))

        return assets;
    }

    asset get_payout_extension_cost(const comment_object &input_comment, const fc::time_point_sec &input_time) const {
        FC_ASSERT(
                (input_time - fc::time_point::now()).to_seconds() /
                (3600 * 24) >
                0, "Extension time should be equal or greater than a day");
        FC_ASSERT((input_time - fc::time_point::now()).to_seconds() <
                  STEEMIT_CASHOUT_WINDOW_SECONDS, "Extension time should be less or equal than a week");

        return asset(((input_time - fc::time_point::now()).to_seconds() * STEEMIT_PAYOUT_EXTENSION_COST_PER_DAY / (input_comment.net_rshares * 60 * 60 * 24), SBD_SYMBOL));
    }


    asset to_sbd(const asset &steem) const {
        return utilities::to_sbd(references.get_feed_history().current_median_history, steem);
    }

    asset to_steem(const asset &sbd) const {
        return utilities::to_steem(references.get_feed_history().current_median_history, sbd);
    }

    fc::sha256 get_pow_target() const {
        const auto &dgp = references.get_dynamic_global_properties();
        fc::sha256 target;
        target._hash[0] = -1;
        target._hash[1] = -1;
        target._hash[2] = -1;
        target._hash[3] = -1;
        target = target >> ((dgp.num_pow_witnesses / 4) + 4);
        return target;
    }

    void update_median_feed() {
            try {
                if ((references.head_block_num() % STEEMIT_FEED_INTERVAL_BLOCKS) != 0) {
                    return;
                }

                auto now = references.head_block_time();
                const witness_schedule_object &wso = references.get_witness_schedule_object();
                vector<price> feeds;
                feeds.reserve(wso.num_scheduled_witnesses);
                for (int i = 0; i < wso.num_scheduled_witnesses; i++) {
                    const auto &wit = references.get_witness(wso.current_shuffled_witnesses[i]);
                    if (wit.last_sbd_exchange_update <
                        now + STEEMIT_MAX_FEED_AGE &&
                        !wit.sbd_exchange_rate.is_null()) {
                        feeds.push_back(wit.sbd_exchange_rate);
                    }
                }

                if (feeds.size() >= STEEMIT_MIN_FEEDS) {
                    std::sort(feeds.begin(), feeds.end());
                    auto median_feed = feeds[feeds.size() / 2];

                    references.modify(references.get_feed_history(), [&](feed_history_object &fho) {
                        fho.price_history.push_back(median_feed);
                        size_t steem_feed_history_window = STEEMIT_FEED_HISTORY_WINDOW_PRE_HF16;
                        if (references.has_hardfork(STEEMIT_HARDFORK_0_16__551)) {
                            steem_feed_history_window = STEEMIT_FEED_HISTORY_WINDOW;
                        }

                        if (fho.price_history.size() >
                            steem_feed_history_window) {
                            fho.price_history.pop_front();
                        }

                        if (fho.price_history.size()) {
                            std::deque<price> copy;
                            for (auto i : fho.price_history) {
                                copy.push_back(i);
                            }

                            std::sort(copy.begin(), copy.end()); /// TODO: use nth_item
                            fho.current_median_history = copy[copy.size() / 2];

#ifdef STEEMIT_BUILD_TESTNET
                            if (skip_price_feed_limit_check) {
                                return;
                            }
#endif
                            if (references.has_hardfork(STEEMIT_HARDFORK_0_14__230)) {
                                const auto &gpo = references.get_dynamic_global_properties();
                                price min_price(asset(9 *
                                                      gpo.current_sbd_supply.amount, SBD_SYMBOL), gpo.current_supply); // This price limits SBD to 10% market cap

                                if (min_price > fho.current_median_history) {
                                    fho.current_median_history = min_price;
                                }
                            }
                        }
                    });
                }
            } FC_CAPTURE_AND_RETHROW()
        }

    uint32_t get_pow_summary_target() const {
        const dynamic_global_property_object &dgp = references.get_dynamic_global_properties();
        if (dgp.num_pow_witnesses >= 1004) {
            return 0;
        }

        if (references.has_hardfork(STEEMIT_HARDFORK_0_16__551)) {
            return (0xFE00 - 0x0040 * dgp.num_pow_witnesses) << 0x10;
        } else {
            return (0xFC00 - 0x0040 * dgp.num_pow_witnesses) << 0x10;
        }
    }


    uint32_t get_slot_at_time(fc::time_point_sec when) const {
        fc::time_point_sec first_slot_time = get_slot_time(1);
        if (when < first_slot_time) {
            return 0;
        }
        return (when - first_slot_time).to_seconds() /
               STEEMIT_BLOCK_INTERVAL + 1;
    }

};
}}
#endif
