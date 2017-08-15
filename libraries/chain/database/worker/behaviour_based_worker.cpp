#include <steemit/chain/database/worker/behaviour_based_worker.hpp>

namespace steemit {
    namespace chain {

        behaviour_based_worker::behaviour_based_worker(database_t &db) : database_worker_t(db, "behaviour_based") {
            add("pay_reward_funds", [&](std::vector<boost::any> args) -> boost::any {
                    share_type reward = boost::any_cast<share_type>(args[0]);
                    const auto &reward_idx = database.get_index<reward_fund_index, by_id>();
                    share_type used_rewards = 0;

                    for (auto itr = reward_idx.begin(); itr != reward_idx.end(); ++itr) {
                        // reward is a per block reward and the percents are 16-bit. This should never overflow
                        auto r = (reward * itr->percent_content_rewards) / STEEMIT_100_PERCENT;

                        database.modify(*itr, [&](reward_fund_object &rfo) {
                            rfo.reward_balance += asset(r, STEEM_SYMBOL);
                        });

                        used_rewards += r;

                        FC_ASSERT(used_rewards <= reward);
                    }

                    return used_rewards;
                }

            );


            ///  Converts STEEM into sbd and adds it to to_account while reducing the STEEM supply
            ///  by STEEM and increasing the sbd supply by the specified amount.

            add("create_sbd", [&](std::vector<boost::any> args) -> boost::any {
                const account_object to_account = boost::any_cast<account_object>(args[0]);
                asset steem = boost::any_cast<asset>(args[1]);
                std::pair<asset, asset> assets(asset(0, SBD_SYMBOL), asset(0, STEEM_SYMBOL));

                try {
                    if (steem.amount == 0) {
                        return assets;
                    }

                    const auto &median_price = database.get_feed_history().current_median_history;
                    const auto &gpo = database.get_dynamic_global_properties();

                    if (!median_price.is_null()) {
                        auto to_sbd = (gpo.sbd_print_rate * steem.amount) / STEEMIT_100_PERCENT;
                        auto to_steem = steem.amount - to_sbd;

                        auto sbd = asset(to_sbd, STEEM_SYMBOL) * median_price;

                        database.dynamic_extension_worker().get("account")->invoke("adjust_balance", to_account, sbd);
                        database.dynamic_extension_worker().get("account")->invoke("adjust_balance", to_account,
                                                                                   asset(to_steem, STEEM_SYMBOL));
                        database.dynamic_extension_worker().get("asset")->invoke("adjust_supply",
                                                                                 asset(-to_sbd, STEEM_SYMBOL));
                        database.dynamic_extension_worker().get("asset")->invoke("adjust_supply", sbd);
                        assets.first = sbd;
                        assets.second = to_steem;
                    } else {
                        database.dynamic_extension_worker().get("account")->invoke("adjust_balance", to_account, steem);
                        assets.second = steem;
                    }
                } FC_CAPTURE_LOG_AND_RETHROW((to_account.name)(steem))

                return assets;
            });

            add("to_sbd", [&](std::vector<boost::any> args) -> boost::any {
                const asset steem = boost::any_cast<asset>(args[0]);
                return utilities::to_sbd(database.get_feed_history().current_median_history, steem);
            });

            add("to_steem", [&](std::vector<boost::any> args) -> boost::any {
                const asset sbd = boost::any_cast<asset>(args[0]);
                return utilities::to_steem(database.get_feed_history().current_median_history, sbd);
            });
        }
    }
}