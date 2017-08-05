#include <steemit/chain/database/worker/account_worker.hpp>
namespace steemit {
    namespace chain {

        account_worker::account_worker(database_tag &db) : database_worker_t(db,"account") {
            add("adjust_balance", [&](std::vector<boost::any>args) -> boost::any {
                const account_object a = boost::any_cast<account_object >(args[0]);
                const asset delta=boost::any_cast<asset &&>(args[1]);

                database.modify(a, [&](account_object &acnt) {
                        switch (delta.symbol) {
                            case STEEM_SYMBOL:
                                acnt.balance += delta;
                                break;
                            case SBD_SYMBOL:
                                if (a.sbd_seconds_last_update != database.head_block_time()) {
                                    acnt.sbd_seconds += fc::uint128_t(a.sbd_balance.amount.value) * (database.head_block_time() - a.sbd_seconds_last_update).to_seconds();
                                    acnt.sbd_seconds_last_update = database.head_block_time();

                                    if (acnt.sbd_seconds > 0 && (acnt.sbd_seconds_last_update - acnt.sbd_last_interest_payment).to_seconds() > STEEMIT_SBD_INTEREST_COMPOUND_INTERVAL_SEC) {
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
                                                    props.virtual_supply += interest_paid * database.get_feed_history().current_median_history;
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

            );

        }
}}
