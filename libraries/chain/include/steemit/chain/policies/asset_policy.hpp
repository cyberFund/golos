#ifndef GOLOS_ASSET_OBJECT_POLICY_HPP
#define GOLOS_ASSET_OBJECT_POLICY_HPP

#include "generic_policy.hpp"

namespace steemit {
namespace chain {
struct asset_policy: public generic_policy {
public:
    asset_policy() = default;

    asset_policy(const asset_policy &) = default;

    asset_policy &operator=(const asset_policy &) = default;

    asset_policy(asset_policy &&) = default;

    asset_policy &operator=(asset_policy &&) = default;

    virtual ~asset_policy() = default;

    asset_policy(database_basic &ref, evaluator_registry <operation> &evaluator_registry_) : generic_policy(ref) {

    }

    void adjust_supply(const asset &delta, bool adjust_vesting) {

        const auto &props = get_dynamic_global_properties();
        if (props.head_block_number < STEEMIT_BLOCKS_PER_DAY * 7) {
            adjust_vesting = false;
        }

        references.modify(props, [&](dynamic_global_property_object &props) {
            switch (delta.symbol) {
                case STEEM_SYMBOL: {
                    asset new_vesting((adjust_vesting && delta.amount > 0) ?
                                      delta.amount * 9 : 0, STEEM_SYMBOL);
                    props.current_supply += delta + new_vesting;
                    props.virtual_supply += delta + new_vesting;
                    props.total_vesting_fund_steem += new_vesting;
                    assert(props.current_supply.amount.value >= 0);
                    break;
                }
                case SBD_SYMBOL:
                    props.current_sbd_supply += delta;
                    props.virtual_supply = props.current_sbd_supply *
                                           get_feed_history().current_median_history +
                                           props.current_supply;
                    assert(props.current_sbd_supply.amount.value >= 0);
                    break;
                default:
                    FC_ASSERT(false, "invalid symbol");
            }
        });
    }


    /**
*  Iterates over all conversion requests with a conversion date before
*  the head block time and then converts them to/from steem/sbd at the
*  current median price feed history price times the premium
*/
    void process_conversions() {
        auto now = head_block_time();
        const auto &request_by_date = get_index<convert_request_index>().indices().get<by_conversion_date>();
        auto itr = request_by_date.begin();

        const auto &fhistory = references.get_feed_history();
        if (fhistory.current_median_history.is_null()) {
            return;
        }

        asset net_sbd(0, SBD_SYMBOL);
        asset net_steem(0, STEEM_SYMBOL);

        while (itr != request_by_date.end() &&
               itr->conversion_date <= now) {
            const auto &user = get_account(itr->owner);
            auto amount_to_issue =
                    itr->amount * fhistory.current_median_history;

            adjust_balance(user, amount_to_issue);

            net_sbd += itr->amount;
            net_steem += amount_to_issue;

            references.push_virtual_operation(fill_convert_request_operation(user.name, itr->requestid, itr->amount, amount_to_issue));

            references.remove(*itr);
            itr = request_by_date.begin();
        }

        const auto &props = references.get_dynamic_global_properties();
        references.modify(props, [&](dynamic_global_property_object &p) {
            p.current_supply += net_steem;
            p.current_sbd_supply -= net_sbd;
            p.virtual_supply += net_steem;
            p.virtual_supply -=
                    net_sbd * references.get_feed_history().current_median_history;
        });
    }

};
}}
#endif //GOLOS_ASSET_OBJECT_POLICY_HPP
