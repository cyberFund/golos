#include <steemit/chain/database/worker/asset_worker.hpp>

namespace steemit {
    namespace chain {


        asset_worker::asset_worker(database_tag &db) : database_worker_t(db, "asset") {

            add("adjust_supply", [&](std::vector<boost::any> args) -> boost::any {

                    const asset delta = boost::any_cast<asset>(args[0]);
                    bool adjust_vesting = boost::any_cast<bool>(args[1]);

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
            );
        }
    }
}