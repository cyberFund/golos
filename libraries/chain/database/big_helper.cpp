#include <steemit/chain/database/big_helper.hpp>
#include <steemit/chain/database/database_basic.hpp>
#include <steemit/chain/utilities/asset.hpp>
#include <steemit/chain/hardfork.hpp>

namespace steemit {
    namespace chain {
        namespace database_helper {
            namespace big_helper {

                const account_object &
                get_account(database_basic &db, const account_name_type &name) {
                    try {
                        return db.get<account_object, by_name>(name);
                    } FC_CAPTURE_AND_RETHROW((name))
                }

                const witness_schedule_object &get_witness_schedule_object(database_basic &db) {
                    try {
                        return db.get<witness_schedule_object>();
                    } FC_CAPTURE_AND_RETHROW()
                }


                account_name_type get_scheduled_witness(database_basic & db, uint32_t slot_num) {
                const dynamic_global_property_object &dpo = db.get_dynamic_global_properties();
                const witness_schedule_object &wso = get_witness_schedule_object(db);
                uint64_t current_aslot = dpo.current_aslot + slot_num;
                return wso.current_shuffled_witnesses[current_aslot % wso.num_scheduled_witnesses];
            }

                const witness_object &get_witness(database_basic &db, const account_name_type &name) {
                    try {
                        return db.get<witness_object, by_name>(name);
                    } FC_CAPTURE_AND_RETHROW((name))
                }

                void adjust_balance(database_basic &database, const account_object &a, const asset &delta) {
                    database.modify(a, [&](account_object &acnt) {
                        switch (delta.symbol) {
                            case STEEM_SYMBOL:
                                acnt.balance += delta;
                                break;
                            case SBD_SYMBOL:
                                if (a.sbd_seconds_last_update != database.head_block_time()) {
                                    acnt.sbd_seconds += fc::uint128_t(a.sbd_balance.amount.value) * (database.head_block_time() - a.sbd_seconds_last_update).to_seconds();
                                    acnt.sbd_seconds_last_update = database.head_block_time();

                                    if (acnt.sbd_seconds > 0 &&
                                        (acnt.sbd_seconds_last_update - acnt.sbd_last_interest_payment).to_seconds() >
                                        STEEMIT_SBD_INTEREST_COMPOUND_INTERVAL_SEC) {
                                        auto interest = acnt.sbd_seconds / STEEMIT_SECONDS_PER_YEAR;
                                        interest *= database.get_dynamic_global_properties().sbd_interest_rate;
                                        interest /= STEEMIT_100_PERCENT;
                                        asset interest_paid(interest.to_uint64(), SBD_SYMBOL);
                                        acnt.sbd_balance += interest_paid;
                                        acnt.sbd_seconds = 0;
                                        acnt.sbd_last_interest_payment = database.head_block_time();
                                        database.push_virtual_operation(interest_operation(a.name, interest_paid));

                                        database.modify(
                                                database.get_dynamic_global_properties(),
                                                [&](dynamic_global_property_object &props) {
                                                    props.current_sbd_supply += interest_paid;
                                                    props.virtual_supply += interest_paid *
                                                                            database.get_feed_history().current_median_history;
                                                }
                                        );
                                    }
                                }
                                acnt.sbd_balance += delta;
                                break;
                            default:
                                FC_ASSERT(false, "invalid symbol");
                        }
                    });
                }

                bool update_account_bandwidth(database_basic &database, const account_object &a, uint32_t trx_size,
                                              const bandwidth_type type) {
                    const auto &props = database.get_dynamic_global_properties();
                    bool has_bandwidth = true;

                    if (props.total_vesting_shares.amount > 0) {
                        auto band = database.find<account_bandwidth_object, by_account_bandwidth_type>(
                                boost::make_tuple(a.name, type));

                        if (band == nullptr) {
                            band = &database.create<account_bandwidth_object>([&](account_bandwidth_object &b) {
                                b.account = a.name;
                                b.type = type;
                            });
                        }

                        share_type new_bandwidth;
                        share_type trx_bandwidth = trx_size * STEEMIT_BANDWIDTH_PRECISION;
                        auto delta_time = (database.head_block_time() - band->last_bandwidth_update).to_seconds();

                        if (delta_time > STEEMIT_BANDWIDTH_AVERAGE_WINDOW_SECONDS) {
                            new_bandwidth = 0;
                        } else {
                            new_bandwidth = (((STEEMIT_BANDWIDTH_AVERAGE_WINDOW_SECONDS - delta_time) *
                                              fc::uint128(band->average_bandwidth.value)) /
                                             STEEMIT_BANDWIDTH_AVERAGE_WINDOW_SECONDS).to_uint64();
                        }

                        new_bandwidth += trx_bandwidth;

                        database.modify(
                                *band,
                                [&](account_bandwidth_object &b) {
                                    b.average_bandwidth = new_bandwidth;
                                    b.lifetime_bandwidth += trx_bandwidth;
                                    b.last_bandwidth_update = database.head_block_time();
                                }
                        );

                        fc::uint128 account_vshares(a.vesting_shares.amount.value);
                        fc::uint128 total_vshares(props.total_vesting_shares.amount.value);
                        fc::uint128 account_average_bandwidth(band->average_bandwidth.value);
                        fc::uint128 max_virtual_bandwidth(props.max_virtual_bandwidth);

                        has_bandwidth = (account_vshares * max_virtual_bandwidth) >
                                        (account_average_bandwidth * total_vshares);

                        if (database.

                                is_producing()

                                )
                            FC_ASSERT(has_bandwidth,
                                      "Account exceeded maximum allowed bandwidth per vesting share.",
                                      ("account_vshares", account_vshares)
                                              ("account_average_bandwidth", account_average_bandwidth)
                                              ("max_virtual_bandwidth", max_virtual_bandwidth)
                                              ("total_vesting_shares", total_vshares));
                    }

                    return has_bandwidth;
                }

                void old_update_account_bandwidth(database_basic &database, const account_object &a, uint32_t trx_size,
                                                  const bandwidth_type type) {
                    try {
                        const auto &props = database.get_dynamic_global_properties();
                        if (props.total_vesting_shares.amount > 0) {
                            FC_ASSERT(a
                                              .vesting_shares.amount > 0,
                                      "Only accounts with a postive vesting balance may transact.");

                            auto band = database.find<account_bandwidth_object, by_account_bandwidth_type>(
                                    boost::make_tuple(a.name, type));

                            if (band == nullptr) {
                                band = &database.create<account_bandwidth_object>([&](account_bandwidth_object &b) {
                                    b.account = a.name;
                                    b.type = type;
                                });
                            }

                            database.
                                    modify(
                                    *band,
                                    [&](account_bandwidth_object &b) {
                                        b.lifetime_bandwidth += trx_size * STEEMIT_BANDWIDTH_PRECISION;

                                        auto now = database.head_block_time();
                                        auto delta_time = (now - b.last_bandwidth_update).to_seconds();
                                        uint64_t N = trx_size * STEEMIT_BANDWIDTH_PRECISION;
                                        if (delta_time >= STEEMIT_BANDWIDTH_AVERAGE_WINDOW_SECONDS) {
                                            b.average_bandwidth = N;
                                        } else {
                                            auto old_weight = b.average_bandwidth *
                                                              (STEEMIT_BANDWIDTH_AVERAGE_WINDOW_SECONDS - delta_time);
                                            auto new_weight = delta_time * N;
                                            b.average_bandwidth = (old_weight + new_weight) /
                                                                  STEEMIT_BANDWIDTH_AVERAGE_WINDOW_SECONDS;
                                        }

                                        b.last_bandwidth_update = now;
                                    }
                            );

                            fc::uint128 account_vshares(a.effective_vesting_shares().amount.value);
                            fc::uint128 total_vshares(props.total_vesting_shares.amount.value);

                            fc::uint128 account_average_bandwidth(band->average_bandwidth.value);
                            fc::uint128 max_virtual_bandwidth(props.max_virtual_bandwidth);

                            FC_ASSERT((account_vshares
                                       * max_virtual_bandwidth) >
                                      (
                                              account_average_bandwidth * total_vshares
                                      ),
                                      "Account exceeded maximum allowed bandwidth per vesting share.",
                                      ("account_vshares", account_vshares)
                                              ("account_average_bandwidth", account_average_bandwidth)
                                              ("max_virtual_bandwidth", max_virtual_bandwidth)
                                              ("total_vesting_shares", total_vshares));
                        }
                    }
                    FC_CAPTURE_AND_RETHROW()

                }

                void adjust_supply(database_basic &database, const asset &delta, bool adjust_vesting) {
                    const auto &props = database.get_dynamic_global_properties();
                    if (props.head_block_number < STEEMIT_BLOCKS_PER_DAY * 7) {
                        adjust_vesting = false;
                    }

                    database.modify(props, [&](dynamic_global_property_object &props) {
                        switch (delta.symbol) {
                            case STEEM_SYMBOL: {
                                asset new_vesting((adjust_vesting && delta.amount > 0) ? delta.amount * 9 : 0,
                                                  STEEM_SYMBOL);
                                props.current_supply += delta + new_vesting;
                                props.virtual_supply += delta + new_vesting;
                                props.total_vesting_fund_steem += new_vesting;
                                assert(props.current_supply.amount.value >= 0);
                                break;
                            }
                            case SBD_SYMBOL:
                                props.current_sbd_supply += delta;
                                props.virtual_supply =
                                        props.current_sbd_supply * database.get_feed_history().current_median_history +
                                        props.current_supply;
                                assert(props.current_sbd_supply.amount.value >= 0);
                                break;
                            default:
                                FC_ASSERT(false, "invalid symbol");
                        }
                    });
                }

                share_type pay_reward_funds(database_basic &database, share_type reward) {
                    const auto &reward_idx = database.get_index<reward_fund_index, by_id>();
                    share_type used_rewards = 0;

                    for (auto itr = reward_idx.begin(); itr != reward_idx.end(); ++itr) {
                        // reward is a per block reward and the percents are 16-bit. This should never overflow
                        auto r = (reward * itr->percent_content_rewards) / STEEMIT_100_PERCENT;

                        database.
                                modify(*itr,
                                       [&](reward_fund_object &rfo) {
                                           rfo.reward_balance += asset(r, STEEM_SYMBOL);
                                       }
                        );

                        used_rewards += r;

                        FC_ASSERT(used_rewards <= reward);
                    }

                    return used_rewards;
                }

                std::pair<asset, asset>
                create_sbd(database_basic &database, const account_object &to_account, asset steem) {
                    std::pair<asset, asset> assets(asset(0, SBD_SYMBOL), asset(0, STEEM_SYMBOL));

                    try {
                        if (steem.amount == 0) {
                            return
                                    assets;
                        }

                        const auto &median_price = database.get_feed_history().current_median_history;
                        const auto &gpo = database.get_dynamic_global_properties();

                        if (!median_price.is_null()) {
                            auto to_sbd = (gpo.sbd_print_rate * steem.amount) / STEEMIT_100_PERCENT;
                            auto to_steem = steem.amount - to_sbd;

                            auto sbd = asset(to_sbd, STEEM_SYMBOL) * median_price;

                            adjust_balance(database,to_account, sbd);
                            adjust_balance(database,to_account, asset(to_steem, STEEM_SYMBOL));
                            adjust_supply(database,asset(-to_sbd, STEEM_SYMBOL));
                            adjust_supply(database,sbd);
                            assets.first = sbd;
                            assets.second = to_steem;
                        } else {
                            adjust_balance(database,to_account, steem);
                            assets.second = steem;
                        }
                    }
                    FC_CAPTURE_LOG_AND_RETHROW((to_account.name)(steem))

                    return assets;
                }

                asset to_sbd(database_basic &database, const asset &steem) {
                    return utilities::to_sbd(database.get_feed_history().current_median_history, steem);
                }

                asset to_steem(database_basic &database, const asset &sbd) {
                    return utilities::to_steem(database.get_feed_history().current_median_history, sbd);
                }

                void adjust_liquidity_reward(database_basic &database, const account_object &owner, const asset &volume, bool is_sdb) {
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
                }

                void adjust_witness_vote(database_basic &database, const witness_object &witness, share_type delta) {
                    database.modify(
                            witness,
                            [&](witness_object &w) {
                                const witness_schedule_object &wso = get_witness_schedule_object(database);
                                auto delta_pos = w.votes.value * (wso.current_virtual_time - w.virtual_last_update);
                                w.virtual_position += delta_pos;

                                w.virtual_last_update = wso.current_virtual_time;
                                w.votes += delta;
                                FC_ASSERT(
                                        w.votes <= database.get_dynamic_global_properties().total_vesting_shares.amount,
                                        "",
                                        ("w.votes", w.votes)("props", database.get_dynamic_global_properties().total_vesting_shares)
                                );

                                if (database.has_hardfork(STEEMIT_HARDFORK_0_2)) {
                                    w.virtual_scheduled_time = w.virtual_last_update +
                                                               (VIRTUAL_SCHEDULE_LAP_LENGTH2 - w.virtual_position) /
                                                               (w.votes.value + 1);
                                } else {
                                    w.virtual_scheduled_time = w.virtual_last_update +
                                                               (VIRTUAL_SCHEDULE_LAP_LENGTH - w.virtual_position) /
                                                               (w.votes.value + 1);
                                }

/** witnesses with a low number of votes could overflow the time field and end up with a scheduled time in the past */
                                if (database.has_hardfork(STEEMIT_HARDFORK_0_4)) {
                                    if (w.virtual_scheduled_time < wso.current_virtual_time) {
                                        w.virtual_scheduled_time = fc::uint128::max_value();
                                    }
                                }
                            });
                }

                void adjust_witness_votes(database_basic &database, const account_object &a, share_type delta) {
                    const auto &vidx = database.get_index<witness_vote_index>().indices().get<by_account_witness>();
                    auto itr = vidx.lower_bound(boost::make_tuple(a.id, witness_id_type()));
                    while (itr != vidx.end() && itr->account == a.id) {
                        adjust_witness_vote(database, database.get(itr->witness), delta);
                        ++itr;
                    }
                }

                void adjust_proxied_witness_votes(database_basic &database, const account_object &a, share_type delta,
                                                  int depth) {
                    if (a.proxy != STEEMIT_PROXY_TO_SELF_ACCOUNT) {
                        /// nested proxies are not supported, vote will not propagate
                        if (depth >= STEEMIT_MAX_PROXY_RECURSION_DEPTH) {
                            return;
                        }

                        const auto &proxy = get_account(database,a.proxy);

                        database.modify(proxy, [&](account_object &a) {
                            a.proxied_vsf_votes[depth] += delta;
                        });

                        adjust_proxied_witness_votes(database, proxy, delta, depth + 1);
                    } else {
                        adjust_witness_votes(database, a, delta);
                    }
                }

                void adjust_proxied_witness_votes(database_basic &database, const account_object &a,
                                                  const std::array<share_type,
                                                          STEEMIT_MAX_PROXY_RECURSION_DEPTH + 1> &delta, int depth) {
                    if (a.proxy != STEEMIT_PROXY_TO_SELF_ACCOUNT) {
                        /// nested proxies are not supported, vote will not propagate
                        if (depth >= STEEMIT_MAX_PROXY_RECURSION_DEPTH) {
                            return;
                        }

                        const auto &proxy = get_account(database,a.proxy);

                        database.modify(proxy, [&](account_object &a) {
                            for (int i = STEEMIT_MAX_PROXY_RECURSION_DEPTH - depth - 1;
                                 i >= 0; --i) {
                                a.proxied_vsf_votes[i + depth] += delta[i];
                            }
                        });

                        adjust_proxied_witness_votes(database, proxy, delta, depth + 1);
                    } else {
                        share_type total_delta = 0;
                        for (int i = STEEMIT_MAX_PROXY_RECURSION_DEPTH - depth;
                             i >= 0; --i) {
                            total_delta += delta[i];
                        }
                        adjust_witness_votes(database, a, total_delta);
                    }
                }

                asset create_vesting(database_basic &database, const account_object &to_account, asset steem) {
                    try {
                        const auto &cprops = database.get_dynamic_global_properties();
                        /**
                        *  The ratio of total_vesting_shares / total_vesting_fund_steem should not
                        *  change as the result of the user adding funds
                        *
                        *  V / C  = (V+Vn) / (C+Cn)
                        *
                        *  Simplifies to Vn = (V * Cn ) / C
                        *
                        *  If Cn equals o.amount, then we must solve for Vn to know how many new vesting shares
                        *  the user should receive.
                        *
                        *  128 bit math is requred due to multiplying of 64 bit numbers. This is done in asset and price.
                        */
                        asset new_vesting = steem * cprops.get_vesting_share_price();

                        database.modify(to_account, [&](account_object &to) {
                            to.vesting_shares += new_vesting;
                        });

                        database.modify(cprops, [&](dynamic_global_property_object &props) {
                            props.total_vesting_fund_steem += steem;
                            props.total_vesting_shares += new_vesting;
                        });

                        adjust_proxied_witness_votes(database, to_account, new_vesting.amount);

                        return new_vesting;
                    }
                    FC_CAPTURE_AND_RETHROW((to_account.name)(steem))
                }


            }
    }
}}

