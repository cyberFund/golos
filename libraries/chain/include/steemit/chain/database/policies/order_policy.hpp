#ifndef GOLOS_LIMIT_ORDER_OBJECT_POLICY_HPP
#define GOLOS_LIMIT_ORDER_OBJECT_POLICY_HPP

#include <steemit/chain/database/generic_policy.hpp>
#include <steemit/chain/chain_objects/steem_objects.hpp>

namespace steemit {
    namespace chain {
        struct order_policy : public generic_policy {
            order_policy(const order_policy &) = default;

            order_policy &operator=(const order_policy &) = default;

            order_policy(order_policy &&) = default;

            order_policy &operator=(order_policy &&) = default;

            virtual ~order_policy() = default;

            order_policy(database_basic &ref, int);

            void clear_expired_orders();

            bool apply_order(const limit_order_object &new_order_object);

            bool fill_order(const limit_order_object &order, const asset &pays, const asset &receives);

            void cancel_order(const limit_order_object &order);

            int
            match(const limit_order_object &new_order, const limit_order_object &old_order, const price &match_price);

            const limit_order_object &get_limit_order(const account_name_type &name, uint32_t orderid) const;

            const limit_order_object *find_limit_order(const account_name_type &name, uint32_t orderid) const;

        };
    }
}
#endif