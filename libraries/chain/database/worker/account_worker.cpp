#include <steemit/chain/database/worker/account_worker.hpp>

namespace steemit {
    namespace chain {

        account_worker::account_worker(database_tag &db) : database_worker_t(db, "account") {
            add("adjust_balance", [&](std::vector<boost::any> args) -> boost::any {
                const account_object a = boost::any_cast<account_object>(args[0]);
                const asset delta = boost::any_cast<asset &&>(args[1]);

                database.modify(a, [&](account_object &acnt) {
                    switch (delta.symbol) {
                        case STEEM_SYMBOL:
                            acnt.balance += delta;
                            break;
                        case SBD_SYMBOL:
                            if (a.sbd_seconds_last_update != database.head_block_time()) {
                                acnt.sbd_seconds += fc::uint128_t(a.sbd_balance.amount.value) *
                                                    (database.head_block_time() -
                                                     a.sbd_seconds_last_update).to_seconds();
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

                                    database.modify(database.get_dynamic_global_properties(),
                                                    [&](dynamic_global_property_object &props) {
                                                        props.current_sbd_supply += interest_paid;
                                                        props.virtual_supply += interest_paid *
                                                                                database.get_feed_history().current_median_history;
                                                    });
                                }
                            }
                            acnt.sbd_balance += delta;
                            break;
                        default:
                            FC_ASSERT(false, "invalid symbol");
                    }
                });
            });


            add("update_account_bandwidth", [&](std::vector<boost::any> args) -> boost::any {

                const account_object a = boost::any_cast<account_object>(args[0]);
                uint32_t trx_size = boost::any_cast<uint32_t>(args[1]);
                const bandwidth_type type = boost::any_cast<bandwidth_type>(args[2]);

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

                    database.modify(*band, [&](account_bandwidth_object &b) {
                        b.average_bandwidth = new_bandwidth;
                        b.lifetime_bandwidth += trx_bandwidth;
                        b.last_bandwidth_update = database.head_block_time();
                    });

                    fc::uint128 account_vshares(a.vesting_shares.amount.value);
                    fc::uint128 total_vshares(props.total_vesting_shares.amount.value);
                    fc::uint128 account_average_bandwidth(band->average_bandwidth.value);
                    fc::uint128 max_virtual_bandwidth(props.max_virtual_bandwidth);

                    has_bandwidth =
                            (account_vshares * max_virtual_bandwidth) > (account_average_bandwidth * total_vshares);

                    if (database.is_producing())
                        FC_ASSERT(has_bandwidth, "Account exceeded maximum allowed bandwidth per vesting share.",
                                  ("account_vshares", account_vshares)("account_average_bandwidth",
                                                                       account_average_bandwidth)(
                                          "max_virtual_bandwidth", max_virtual_bandwidth)("total_vesting_shares",
                                                                                          total_vshares));
                }

                return has_bandwidth;
            });

            add("old_update_account_bandwidth", [&](std::vector<boost::any> args) -> boost::any {
                const account_object a = boost::any_cast<account_object>(args[0]);
                uint32_t trx_size = boost::any_cast<uint32_t>(args[1]);
                const bandwidth_type type = boost::any_cast<bandwidth_type>(args[2]);
                try {
                    const auto &props = database.get_dynamic_global_properties();
                    if (props.total_vesting_shares.amount > 0) {
                        FC_ASSERT(a.vesting_shares.amount > 0,
                                  "Only accounts with a postive vesting balance may transact.");

                        auto band = database.find<account_bandwidth_object, by_account_bandwidth_type>(
                                boost::make_tuple(a.name, type));

                        if (band == nullptr) {
                            band = &database.create<account_bandwidth_object>([&](account_bandwidth_object &b) {
                                b.account = a.name;
                                b.type = type;
                            });
                        }

                        database.modify(*band, [&](account_bandwidth_object &b) {
                            b.lifetime_bandwidth += trx_size * STEEMIT_BANDWIDTH_PRECISION;

                            auto now = database.head_block_time();
                            auto delta_time = (now - b.last_bandwidth_update).to_seconds();
                            uint64_t N = trx_size * STEEMIT_BANDWIDTH_PRECISION;
                            if (delta_time >= STEEMIT_BANDWIDTH_AVERAGE_WINDOW_SECONDS) {
                                b.average_bandwidth = N;
                            } else {
                                auto old_weight =
                                        b.average_bandwidth * (STEEMIT_BANDWIDTH_AVERAGE_WINDOW_SECONDS - delta_time);
                                auto new_weight = delta_time * N;
                                b.average_bandwidth =
                                        (old_weight + new_weight) / STEEMIT_BANDWIDTH_AVERAGE_WINDOW_SECONDS;
                            }

                            b.last_bandwidth_update = now;
                        });

                        fc::uint128 account_vshares(a.effective_vesting_shares().amount.value);
                        fc::uint128 total_vshares(props.total_vesting_shares.amount.value);

                        fc::uint128 account_average_bandwidth(band->average_bandwidth.value);
                        fc::uint128 max_virtual_bandwidth(props.max_virtual_bandwidth);

                        FC_ASSERT(
                                (account_vshares * max_virtual_bandwidth) > (account_average_bandwidth * total_vshares),
                                "Account exceeded maximum allowed bandwidth per vesting share.",
                                ("account_vshares", account_vshares)("account_average_bandwidth",
                                                                     account_average_bandwidth)("max_virtual_bandwidth",
                                                                                                max_virtual_bandwidth)(
                                        "total_vesting_shares", total_vshares));
                    }
                } FC_CAPTURE_AND_RETHROW()
            });


        }
    }
}
