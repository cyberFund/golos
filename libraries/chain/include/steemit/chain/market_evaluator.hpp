#pragma once

#include <steemit/chain/evaluator.hpp>

#include <steemit/protocol/market_operations.hpp>
#include <steemit/protocol/types.hpp>

namespace steemit {
    namespace chain {

        class account_object;

        class asset_object;

        class asset_bitasset_data_object;

        class call_order_object;

        class limit_order_create_evaluator
                : public evaluator_impl<limit_order_create_evaluator> {
        public:
            typedef limit_order_create_operation operation_type;

            limit_order_create_evaluator(database &db)
                    : evaluator_impl<limit_order_create_evaluator>(db) {

            }

            void do_apply(const limit_order_create_operation &o);

            asset calculate_market_fee(const asset_object *aobj, const asset &trade_amount);

        protected:
            share_type _deferred_fee = 0;
            const limit_order_create_operation *_op = nullptr;
            const account_object *_seller = nullptr;
            const asset_object *_sell_asset = nullptr;
            const asset_object *_receive_asset = nullptr;
        };

        class limit_order_cancel_evaluator
                : public evaluator_impl<limit_order_cancel_evaluator> {
        public:
            typedef limit_order_cancel_operation operation_type;

            limit_order_cancel_evaluator(database &db)
                    : evaluator_impl<limit_order_cancel_evaluator>(db) {

            }

            void do_apply(const limit_order_cancel_operation &o);

        protected:
            const limit_order_object *_order;
        };

        class call_order_update_evaluator
                : public evaluator_impl<call_order_update_evaluator> {
        public:
            typedef call_order_update_operation operation_type;

            call_order_update_evaluator(database &db)
                    : evaluator_impl<call_order_update_evaluator>(db) {

            }

            void do_apply(const call_order_update_operation &o);

        protected:
            bool _closing_order = false;
            const asset_object *_debt_asset = nullptr;
            const account_object *_paying_account = nullptr;
            const call_order_object *_order = nullptr;
            const asset_bitasset_data_object *_bitasset_data = nullptr;
        };
    }
} // steemit::chain
