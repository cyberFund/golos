#include <steemit/chain/database/policies/asset_policy.hpp>
#include <steemit/chain/database/database_basic.hpp>
#include <steemit/chain/steem_objects.hpp>
namespace steemit {
    namespace chain {

        asset_policy::asset_policy(database_basic &ref,int) : generic_policy(ref) {

        }

        void asset_policy::process_conversions() {

            auto now = references.head_block_time();
            const auto &request_by_date = references.get_index<convert_request_index>().indices().get<by_conversion_date>();
            auto itr = request_by_date.begin();

            const auto &fhistory = references.get_feed_history();
            if (fhistory.current_median_history.is_null()) {
                return;
            }

            asset net_sbd(0, SBD_SYMBOL);
            asset net_steem(0, STEEM_SYMBOL);

            while (itr != request_by_date.end() && itr->conversion_date <= now) {
                const auto &user = references.get_account(itr->owner);
                auto amount_to_issue = itr->amount * fhistory.current_median_history;

                references.dynamic_extension_worker().get("account")->invoke("adjust_balance",user, amount_to_issue);

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
                p.virtual_supply -= net_sbd * references.get_feed_history().current_median_history;
            });
        }

        void asset_policy::adjust_supply(const asset &delta, bool adjust_vesting) {
            references.dynamic_extension_worker().get("asset")->invoke("adjust_supply",delta,adjust_vesting);
        }
    }
}