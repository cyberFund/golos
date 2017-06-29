#pragma once

#include <steemit/chain/asset_object.hpp>
#include <steemit/chain/evaluator.hpp>
#include <steemit/chain/database.hpp>

#include <steemit/protocol/operations.hpp>

namespace steemit {
    namespace chain {

        class asset_create_evaluator
                : public evaluator_impl<asset_create_evaluator> {
        public:
            typedef asset_create_operation operation_type;

            asset_create_evaluator(database &db)
                    : evaluator_impl<asset_create_evaluator>(db) {

            }

            void do_apply(const asset_create_operation &o);
        };

        class asset_issue_evaluator
                : public evaluator_impl<asset_issue_evaluator> {
        public:
            typedef asset_issue_operation operation_type;

            asset_issue_evaluator(database &db)
                    : evaluator_impl<asset_issue_evaluator>(db) {

            }

            void do_apply(const asset_issue_operation &o);

            const asset_dynamic_data_object *asset_dyn_data = nullptr;
            const account_object *to_account = nullptr;
        };

        class asset_reserve_evaluator
                : public evaluator_impl<asset_reserve_evaluator> {
        public:
            typedef asset_reserve_operation operation_type;

            asset_reserve_evaluator(database &db)
                    : evaluator_impl<asset_reserve_evaluator>(db) {

            }

            void do_apply(const asset_reserve_operation &o);

            const asset_dynamic_data_object *asset_dyn_data = nullptr;
            const account_object *from_account = nullptr;
        };


        class asset_update_evaluator
                : public evaluator_impl<asset_update_evaluator> {
        public:
            typedef asset_update_operation operation_type;

            asset_update_evaluator(database &db)
                    : evaluator_impl<asset_update_evaluator>(db) {

            }

            void do_apply(const asset_update_operation &o);

            const asset_object *asset_to_update = nullptr;
        };

        class asset_update_bitasset_evaluator
                : public evaluator_impl<asset_update_bitasset_evaluator> {
        public:
            typedef asset_update_bitasset_operation operation_type;

            asset_update_bitasset_evaluator(database &db)
                    : evaluator_impl<asset_update_bitasset_evaluator>(db) {

            }

            void do_apply(const asset_update_bitasset_operation &o);

            const asset_bitasset_data_object *bitasset_to_update = nullptr;
        };

        class asset_update_feed_producers_evaluator
                : public evaluator_impl<asset_update_feed_producers_evaluator> {
        public:
            typedef asset_update_feed_producers_operation operation_type;

            asset_update_feed_producers_evaluator(database &db)
                    : evaluator_impl<asset_update_feed_producers_evaluator>(db) {

            }

            void do_apply(const operation_type &o);

            const asset_bitasset_data_object *bitasset_to_update = nullptr;
        };

        class asset_fund_fee_pool_evaluator
                : public evaluator_impl<asset_fund_fee_pool_evaluator> {
        public:
            typedef asset_fund_fee_pool_operation operation_type;

            asset_fund_fee_pool_evaluator(database &db)
                    : evaluator_impl<asset_fund_fee_pool_evaluator>(db) {

            }

            void do_apply(const asset_fund_fee_pool_operation &op);

            const asset_dynamic_data_object *asset_dyn_data = nullptr;
        };

        class asset_global_settle_evaluator
                : public evaluator_impl<asset_global_settle_evaluator> {
        public:
            typedef asset_global_settle_operation operation_type;

            asset_global_settle_evaluator(database &db)
                    : evaluator_impl<asset_global_settle_evaluator>(db) {

            }

            void do_apply(const operation_type &op);

            const asset_object *asset_to_settle = nullptr;
        };

        class asset_settle_evaluator
                : public evaluator_impl<asset_settle_evaluator> {
        public:
            typedef asset_settle_operation operation_type;

            asset_settle_evaluator(database &db)
                    : evaluator_impl<asset_settle_evaluator>(db) {

            }

            void do_apply(const operation_type &op);

            const asset_object *asset_to_settle = nullptr;
        };

        class asset_publish_feeds_evaluator
                : public evaluator_impl<asset_publish_feeds_evaluator> {
        public:
            typedef asset_publish_feed_operation operation_type;

            asset_publish_feeds_evaluator(database &db)
                    : evaluator_impl<asset_publish_feeds_evaluator>(db) {

            }

            void do_apply(const asset_publish_feed_operation &o);

            std::map<std::pair<asset_id_type, asset_id_type>, price_feed> median_feed_values;
        };

        class asset_claim_fees_evaluator
                : public evaluator_impl<asset_claim_fees_evaluator> {
        public:
            typedef asset_claim_fees_operation operation_type;

            asset_claim_fees_evaluator(database &db)
                    : evaluator_impl<asset_claim_fees_evaluator>(db) {

            }

            void do_apply(const asset_claim_fees_operation &o);
        };
    }
} // steemit::chain