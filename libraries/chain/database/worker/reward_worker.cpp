#include <steemit/chain/database/worker/reward_worker.hpp>
namespace steemit {
    namespace chain {

        reward_worker::reward_worker(database_tag& db) : database_worker_t(db,"reward") {
            add("adjust_liquidity_reward", [&](std::vector<boost::any> args) -> boost::any {
                const account_object owner = boost::any_cast<account_object>(args[0]);
                const asset volume = boost::any_cast<asset>(args[1]);
                bool is_sdb = boost::any_cast<bool>(args[2]);

                const auto &ridx = database.get_index<liquidity_reward_balance_index>().indices().get<by_owner>();
                auto itr = ridx.find(owner.id);
                if (itr != ridx.end()) {
                    database.modify<liquidity_reward_balance_object>(
                            *itr,
                            [&](liquidity_reward_balance_object &r) {
                                if (database.head_block_time() - r.last_update >= STEEMIT_LIQUIDITY_TIMEOUT_SEC) {
                                    r.sbd_volume = 0;
                                    r.steem_volume = 0;
                                    r.weight = 0;
                                }

                                if (is_sdb) {
                                    r.sbd_volume += volume.amount.value;
                                } else {
                                    r.steem_volume += volume.amount.value;
                                }

                                r.update_weight(database.has_hardfork(STEEMIT_HARDFORK_0_10__141));
                                r.last_update = database.head_block_time();
                            });
                } else {
                    database.create<liquidity_reward_balance_object>([&](liquidity_reward_balance_object &r) {
                        r.owner = owner.id;
                        if (is_sdb) {
                            r.sbd_volume = volume.amount.value;
                        } else {
                            r.steem_volume = volume.amount.value;
                        }

                        r.update_weight(database.has_hardfork(STEEMIT_HARDFORK_0_9__141));
                        r.last_update = database.head_block_time();
                    });
                }
            });
        }
    }}
