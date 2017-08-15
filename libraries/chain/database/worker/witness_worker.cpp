#include <steemit/chain/database/worker/witness_worker.hpp>

namespace steemit {
    namespace chain {

        void adjust_witness_vote(database_tag &db, const witness_object &witness, share_type delta) {
            db.modify(witness, [&](witness_object &w) {
                const witness_schedule_object &wso = db.get_witness_schedule_object();
                auto delta_pos = w.votes.value * (wso.current_virtual_time - w.virtual_last_update);
                w.virtual_position += delta_pos;

                w.virtual_last_update = wso.current_virtual_time;
                w.votes += delta;
                FC_ASSERT(w.votes <= db.get_dynamic_global_properties().total_vesting_shares.amount, "",
                          ("w.votes", w.votes)("props", db.get_dynamic_global_properties().total_vesting_shares));

                if (db.has_hardfork(STEEMIT_HARDFORK_0_2)) {
                    w.virtual_scheduled_time = w.virtual_last_update +
                                               (VIRTUAL_SCHEDULE_LAP_LENGTH2 - w.virtual_position) /
                                               (w.votes.value + 1);
                } else {
                    w.virtual_scheduled_time = w.virtual_last_update +
                                               (VIRTUAL_SCHEDULE_LAP_LENGTH - w.virtual_position) / (w.votes.value + 1);
                }

                /** witnesses with a low number of votes could overflow the time field and end up with a scheduled time in the past */
                if (db.has_hardfork(STEEMIT_HARDFORK_0_4)) {
                    if (w.virtual_scheduled_time < wso.current_virtual_time) {
                        w.virtual_scheduled_time = fc::uint128::max_value();
                    }
                }
            });
        }


        void adjust_witness_votes(database_tag &db, const account_object &a, share_type delta) {
            const auto &vidx = db.get_index<witness_vote_index>().indices().get<by_account_witness>();
            auto itr = vidx.lower_bound(boost::make_tuple(a.id, witness_id_type()));
            while (itr != vidx.end() && itr->account == a.id) {
                adjust_witness_vote(db, db.get(itr->witness), delta);
                ++itr;
            }
        }


        /** this updates the votes for all witnesses as a result of account VESTS changing */
        void adjust_proxied_witness_votes(database_tag &db, const account_object &a, share_type delta, int depth = 0) {
            if (a.proxy != STEEMIT_PROXY_TO_SELF_ACCOUNT) {
                /// nested proxies are not supported, vote will not propagate
                if (depth >= STEEMIT_MAX_PROXY_RECURSION_DEPTH) {
                    return;
                }

                const auto &proxy = db.get_account(a.proxy);

                db.modify(proxy, [&](account_object &a) {
                    a.proxied_vsf_votes[depth] += delta;
                });

                adjust_proxied_witness_votes(db, proxy, delta, depth + 1);
            } else {
                adjust_witness_votes(db, a, delta);
            }
        }

        /** this updates the votes for witnesses as a result of account voting proxy changing */
        void adjust_proxied_witness_votes(database_tag &db, const account_object &a,
                                          const std::array<share_type, STEEMIT_MAX_PROXY_RECURSION_DEPTH + 1> &delta,
                                          int depth = 0) {
            if (a.proxy != STEEMIT_PROXY_TO_SELF_ACCOUNT) {
                /// nested proxies are not supported, vote will not propagate
                if (depth >= STEEMIT_MAX_PROXY_RECURSION_DEPTH) {
                    return;
                }

                const auto &proxy = db.get_account(a.proxy);

                db.modify(proxy, [&](account_object &a) {
                    for (int i = STEEMIT_MAX_PROXY_RECURSION_DEPTH - depth - 1; i >= 0; --i) {
                        a.proxied_vsf_votes[i + depth] += delta[i];
                    }
                });

                adjust_proxied_witness_votes(db, proxy, delta, depth + 1);
            } else {
                share_type total_delta = 0;
                for (int i = STEEMIT_MAX_PROXY_RECURSION_DEPTH - depth; i >= 0; --i) {
                    total_delta += delta[i];
                }
                adjust_witness_votes(db, a, total_delta);
            }
        }

        witness_worker::witness_worker(database_tag &db) : database_worker_t(db, "witness") {
            add("create_vesting", [&](std::vector<boost::any> args) -> boost::any {
                const account_object to_account = boost::any_cast<account_object>(args[0]);
                asset steem = boost::any_cast<asset>(args[1]);
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
                } FC_CAPTURE_AND_RETHROW((to_account.name)(steem))
            });

            add("adjust_witness_votes", [&](std::vector<boost::any> args) -> boost::any {
                const account_object a = boost::any_cast<account_object>(args[0]);
                share_type delta = boost::any_cast<share_type>(args[1]);
                const auto &vidx = database.get_index<witness_vote_index>().indices().get<by_account_witness>();
                auto itr = vidx.lower_bound(boost::make_tuple(a.id, witness_id_type()));
                while (itr != vidx.end() && itr->account == a.id) {
                    adjust_witness_vote(database, database.get(itr->witness), delta);
                    ++itr;
                }
            });

            add("adjust_witness_vote", [&](std::vector<boost::any> args) -> boost::any {
                const witness_object witness = boost::any_cast<witness_object>(args[0]);
                share_type delta = boost::any_cast<share_type>(args[1]);
                adjust_witness_vote(database, witness, delta);
            });


            add("adjust_proxied_witness_votes", [&](std::vector<boost::any> args) -> boost::any {
                const account_object a = boost::any_cast<account_object>(args[0]);
                share_type delta = boost::any_cast<share_type>(args[1]);
                int depth = boost::any_cast<int>(args[2]);
                adjust_proxied_witness_votes(database, a, delta, depth);
            });

            add("adjust_proxied_witness_votes_1", [&](std::vector<boost::any> args) -> boost::any {
                const account_object a = boost::any_cast<account_object>(args[0]);
                const std::array<share_type, STEEMIT_MAX_PROXY_RECURSION_DEPTH + 1> delta = boost::any_cast<
                        std::array<share_type, STEEMIT_MAX_PROXY_RECURSION_DEPTH + 1>>(args[1]);
                int depth = boost::any_cast<int>(args[2]);
                adjust_proxied_witness_votes(database, a, delta, depth);
            });

        }
    }
}