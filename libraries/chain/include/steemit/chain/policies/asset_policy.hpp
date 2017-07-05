#ifndef GOLOS_ASSET_OBJECT_POLICY_HPP
#define GOLOS_ASSET_OBJECT_POLICY_HPP
namespace steemit {
namespace chain {
struct asset_policy {

    asset_policy() = default;

    asset_policy(const asset_policy &) = default;

    asset_policy &operator=(const asset_policy &) = default;

    asset_policy(asset_policy &&) = default;

    asset_policy &operator=(asset_policy &&) = default;

    virtual ~asset_policy() = default;

    asset_policy(database_basic &ref, evaluator_registry <operation> &evaluator_registry_) : references(ref) {

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

            references.adjust_balance(user, amount_to_issue);

            net_sbd += itr->amount;
            net_steem += amount_to_issue;

            references.push_virtual_operation(fill_convert_request_operation(user.name, itr->requestid, itr->amount, amount_to_issue));

            remove(*itr);
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

    int match(const limit_order_object &new_order, const limit_order_object &old_order, const price &match_price) {
        assert(new_order.sell_price.quote.symbol ==
               old_order.sell_price.base.symbol);
        assert(new_order.sell_price.base.symbol ==
               old_order.sell_price.quote.symbol);
        assert(new_order.for_sale > 0 && old_order.for_sale > 0);
        assert(match_price.quote.symbol ==
               new_order.sell_price.base.symbol);
        assert(match_price.base.symbol == old_order.sell_price.base.symbol);

        auto new_order_for_sale = new_order.amount_for_sale();
        auto old_order_for_sale = old_order.amount_for_sale();

        asset new_order_pays, new_order_receives, old_order_pays, old_order_receives;

        if (new_order_for_sale <= old_order_for_sale * match_price) {
            old_order_receives = new_order_for_sale;
            new_order_receives = new_order_for_sale * match_price;
        } else {
            //This line once read: assert( old_order_for_sale < new_order_for_sale * match_price );
            //This assert is not always true -- see trade_amount_equals_zero in operation_tests.cpp
            //Although new_order_for_sale is greater than old_order_for_sale * match_price, old_order_for_sale == new_order_for_sale * match_price
            //Removing the assert seems to be safe -- apparently no asset is created or destroyed.
            new_order_receives = old_order_for_sale;
            old_order_receives = old_order_for_sale * match_price;
        }

        old_order_pays = new_order_receives;
        new_order_pays = old_order_receives;

        assert(new_order_pays == new_order.amount_for_sale() ||
               old_order_pays == old_order.amount_for_sale());

        auto age = head_block_time() - old_order.created;
        if (!has_hardfork(STEEMIT_HARDFORK_0_12__178) &&
            ((age >= STEEMIT_MIN_LIQUIDITY_REWARD_PERIOD_SEC &&
              !has_hardfork(STEEMIT_HARDFORK_0_10__149)) ||
             (age >= STEEMIT_MIN_LIQUIDITY_REWARD_PERIOD_SEC_HF10 &&
              has_hardfork(STEEMIT_HARDFORK_0_10__149)))) {
            if (old_order_receives.symbol == STEEM_SYMBOL) {
                references.adjust_liquidity_reward(get_account(old_order.seller), old_order_receives, false);
                references.adjust_liquidity_reward(get_account(new_order.seller), -old_order_receives, false);
            } else {
                references.adjust_liquidity_reward(get_account(old_order.seller), new_order_receives, true);
                references.adjust_liquidity_reward(get_account(new_order.seller), -new_order_receives, true);
            }
        }

        references.push_virtual_operation(fill_order_operation(new_order.seller, new_order.orderid, new_order_pays, old_order.seller, old_order.orderid, old_order_pays));

        int result = 0;
        result |= fill_order(new_order, new_order_pays, new_order_receives);
        result |= fill_order(old_order, old_order_pays, old_order_receives)
                << 1;
        assert(result != 0);
        return result;
    }

protected:
    database_basic &references;

};}}
#endif //GOLOS_ASSET_OBJECT_POLICY_HPP
