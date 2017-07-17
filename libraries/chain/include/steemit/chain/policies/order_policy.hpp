#ifndef GOLOS_LIMIT_ORDER_OBJECT_POLICY_HPP
#define GOLOS_LIMIT_ORDER_OBJECT_POLICY_HPP

#include "generic_policy.hpp"

namespace steemit {
namespace chain {
struct order_policy: public generic_policy {

    order_policy() = default;

    order_policy(const order_policy &) = default;

    order_policy &operator=(const order_policy &) = default;

    order_policy(order_policy &&) = default;

    order_policy &operator=(order_policy &&) = default;

    virtual ~order_policy() = default;

    order_policy(database_basic &ref, evaluator_registry <operation> &evaluator_registry_) : generic_policy(ref) {
        evaluator_registry_.register_evaluator<limit_order_create_evaluator>();
        evaluator_registry_.register_evaluator<limit_order_create2_evaluator>();
        evaluator_registry_.register_evaluator<limit_order_cancel_evaluator>();
    }


    void clear_expired_orders() {
        auto now = references.head_block_time();
        const auto &orders_by_exp = references.get_index<limit_order_index>().indices().get<by_expiration>();
        auto itr = orders_by_exp.begin();
        while (itr != orders_by_exp.end() && itr->expiration < now) {
            cancel_order(*itr);
            itr = orders_by_exp.begin();
        }
    }

    bool apply_order(const limit_order_object &new_order_object) {
        auto order_id = new_order_object.id;

        const auto &limit_price_idx = references.get_index<limit_order_index>().indices().get<by_price>();

        auto max_price = ~new_order_object.sell_price;
        auto limit_itr = limit_price_idx.lower_bound(max_price.max());
        auto limit_end = limit_price_idx.upper_bound(max_price);

        bool finished = false;
        while (!finished && limit_itr != limit_end) {
            auto old_limit_itr = limit_itr;
            ++limit_itr;
            // match returns 2 when only the old order was fully filled. In this case, we keep matching; otherwise, we stop.
            finished = (match(new_order_object, *old_limit_itr, old_limit_itr->sell_price) & 0x1);
        }

        return references.find<limit_order_object>(order_id) == nullptr;
    }

    bool fill_order(const limit_order_object &order, const asset &pays, const asset &receives) {
        try {
            FC_ASSERT(order.amount_for_sale().symbol == pays.symbol);
            FC_ASSERT(pays.symbol != receives.symbol);

            const account_object &seller = references.get_account(order.seller);

                adjust_balance(seller, receives);

            if (pays == order.amount_for_sale()) {
                references.remove(order);
                return true;
            } else {
                references.modify(order, [&](limit_order_object &b) {
                    b.for_sale -= pays.amount;
                });
                /**
      *  There are times when the AMOUNT_FOR_SALE * SALE_PRICE == 0 which means that we
      *  have hit the limit where the seller is asking for nothing in return.  When this
      *  happens we must refund any balance back to the seller, it is too small to be
      *  sold at the sale price.
      */
                if (order.amount_to_receive().amount == 0) {
                    cancel_order(order);
                    return true;
                }
                return false;
            }
        }
        FC_CAPTURE_AND_RETHROW((order)(pays)(receives))
    }

    void cancel_order(const limit_order_object &order) {
        adjust_balance(references.get_account(order.seller), order.amount_for_sale());
        references.remove(order);
    }


    int match(const limit_order_object &new_order, const limit_order_object &old_order, const price &match_price) {
        assert(new_order.sell_price.quote.symbol == old_order.sell_price.base.symbol);
        assert(new_order.sell_price.base.symbol == old_order.sell_price.quote.symbol);
        assert(new_order.for_sale > 0 && old_order.for_sale > 0);
        assert(match_price.quote.symbol == new_order.sell_price.base.symbol);
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

        assert(new_order_pays == new_order.amount_for_sale() || old_order_pays == old_order.amount_for_sale());

        auto age = references.head_block_time() - old_order.created;
        if (!references.has_hardfork(STEEMIT_HARDFORK_0_12__178) &&
            ((age >= STEEMIT_MIN_LIQUIDITY_REWARD_PERIOD_SEC &&
              !references.has_hardfork(STEEMIT_HARDFORK_0_10__149)) ||
             (age >= STEEMIT_MIN_LIQUIDITY_REWARD_PERIOD_SEC_HF10 &&
                     references.has_hardfork(STEEMIT_HARDFORK_0_10__149)))) {
            if (old_order_receives.symbol == STEEM_SYMBOL) {
                adjust_liquidity_reward(references.get_account(old_order.seller), old_order_receives, false);
                adjust_liquidity_reward(references.get_account(new_order.seller), -old_order_receives, false);
            } else {
                adjust_liquidity_reward(references.get_account(old_order.seller), new_order_receives, true);
                adjust_liquidity_reward(references.get_account(new_order.seller), -new_order_receives, true);
            }
        }

        references.push_virtual_operation(fill_order_operation(new_order.seller, new_order.orderid, new_order_pays, old_order.seller, old_order.orderid, old_order_pays));

        int result = 0;
        result |= fill_order(new_order, new_order_pays, new_order_receives);
        result |= fill_order(old_order, old_order_pays, old_order_receives) << 1;
        assert(result != 0);
        return result;
    }
};
}}
#endif