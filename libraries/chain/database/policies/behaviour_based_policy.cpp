#include <steemit/chain/database/policies/behaviour_based_policy.hpp>
#include <steemit/chain/database/database_basic.hpp>
#include <steemit/chain/chain_objects/steem_objects.hpp>

namespace steemit {
    namespace chain {

        namespace {
            const witness_schedule_object &get_witness_schedule_object(database_basic &db) {
                try {
                    return db.get<witness_schedule_object>();
                } FC_CAPTURE_AND_RETHROW()
            }

            const witness_object &get_witness(database_basic &db, const account_name_type &name) {
                try {
                    return db.get<witness_object, by_name>(name);
                } FC_CAPTURE_AND_RETHROW((name))
            }
        }

        behaviour_based_policy::behaviour_based_policy(database_basic &ref, int) : generic_policy(ref) {
        }

        protocol::asset behaviour_based_policy::get_payout_extension_cost(const comment_object &input_comment,
                                                                          const fc::time_point_sec &input_time) const {
            FC_ASSERT((input_time - fc::time_point::now()).to_seconds() / (3600 * 24) > 0,
                      "Extension time should be equal or greater than a day");
            FC_ASSERT((input_time - fc::time_point::now()).to_seconds() < STEEMIT_CASHOUT_WINDOW_SECONDS,
                      "Extension time should be less or equal than a week");

            return asset(((input_time - fc::time_point::now()).to_seconds() * STEEMIT_PAYOUT_EXTENSION_COST_PER_DAY /
                          (input_comment.net_rshares * 60 * 60 * 24), SBD_SYMBOL));
        }

        fc::sha256 behaviour_based_policy::get_pow_target() const {
            const auto &dgp = references.get_dynamic_global_properties();
            fc::sha256 target;
            target._hash[0] = -1;
            target._hash[1] = -1;
            target._hash[2] = -1;
            target._hash[3] = -1;
            target = target >> ((dgp.num_pow_witnesses / 4) + 4);
            return target;
        }

        void behaviour_based_policy::update_median_feed() {
            try {
                if ((references.head_block_num() % STEEMIT_FEED_INTERVAL_BLOCKS) != 0) {
                    return;
                }

                auto now = references.head_block_time();
                const witness_schedule_object &wso = get_witness_schedule_object(references);
                vector<price> feeds;
                feeds.reserve(wso.num_scheduled_witnesses);
                for (int i = 0; i < wso.num_scheduled_witnesses; i++) {
                    const auto &wit = get_witness(references, wso.current_shuffled_witnesses[i]);
                    if (wit.last_sbd_exchange_update < now + STEEMIT_MAX_FEED_AGE && !wit.sbd_exchange_rate.is_null()) {
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

                        if (fho.price_history.size() > steem_feed_history_window) {
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
                                price min_price(asset(9 * gpo.current_sbd_supply.amount, SBD_SYMBOL),
                                                gpo.current_supply); // This price limits SBD to 10% market cap

                                if (min_price > fho.current_median_history) {
                                    fho.current_median_history = min_price;
                                }
                            }
                        }
                    });
                }
            } FC_CAPTURE_AND_RETHROW()
        }

        uint32_t behaviour_based_policy::get_pow_summary_target() const {
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
    }
}