#pragma once

#include <golos/chain/objects/asset_object.hpp>
#include <golos/chain/evaluator.hpp>
#include <golos/chain/database.hpp>

#include <golos/protocol/operations/operations.hpp>

namespace golos {
    namespace chain {
        /**
         *  @brief Evaluator for asset_create_operation
         *  @tparam Major Indicates the major protocol version this operation will be used for
         *  @tparam Hardfork Indicates the hardfork protocol version this operation will be used for
         *  @tparam Release Indicates the release protocol version this operation will be used for
         *
         *  @note In fact business logic protocol versioning scheme is not bounded in any way to business logic chain versioning, but it was decided to make them coincide
         *  @see evaluator
         *  @see asset_create_operation
         */
        template<uint8_t Major, uint8_t Hardfork, uint16_t Release>
        class asset_create_evaluator : public evaluator<asset_create_evaluator<Major, Hardfork, Release>, Major,
                Hardfork, Release> {
        public:
            typedef asset_create_operation<Major, Hardfork, Release> operation_type;

            asset_create_evaluator(database &db) : evaluator<asset_create_evaluator<Major, Hardfork, Release>, Major,
                    Hardfork, Release>(db) {

            }

            void do_apply(const operation_type &o);
        };

        /**
         *  @brief Evaluator for asset_issue_operation
         *  @tparam Major Indicates the major protocol version this operation will be used for
         *  @tparam Hardfork Indicates the hardfork protocol version this operation will be used for
         *  @tparam Release Indicates the release protocol version this operation will be used for
         *
         *  @note In fact business logic protocol versioning scheme is not bounded in any way to business logic chain versioning, but it was decided to make them coincide
         *  @see evaluator
         *  @see asset_issue_operation
         */
        template<uint8_t Major, uint8_t Hardfork, uint16_t Release>
        class asset_issue_evaluator : public evaluator<asset_issue_evaluator<Major, Hardfork, Release>, Major, Hardfork,
                Release> {
        public:
            typedef asset_issue_operation<Major, Hardfork, Release> operation_type;

            asset_issue_evaluator(database &db) : evaluator<asset_issue_evaluator<Major, Hardfork, Release>, Major,
                    Hardfork, Release>(db) {

            }

            void do_apply(const operation_type &o);

            const asset_dynamic_data_object *asset_dyn_data = nullptr;
            const account_object *to_account = nullptr;
        };

        /**
         *  @brief Evaluator for asset_reserve_operation
         *  @tparam Major Indicates the major protocol version this operation will be used for
         *  @tparam Hardfork Indicates the hardfork protocol version this operation will be used for
         *  @tparam Release Indicates the release protocol version this operation will be used for
         *
         *  @note In fact business logic protocol versioning scheme is not bounded in any way to business logic chain versioning, but it was decided to make them coincide
         *  @see evaluator
         *  @see asset_reserve_operation
         */
        template<uint8_t Major, uint8_t Hardfork, uint16_t Release>
        class asset_reserve_evaluator : public evaluator<asset_reserve_evaluator<Major, Hardfork, Release>, Major,
                Hardfork, Release> {
        public:
            typedef asset_reserve_operation<Major, Hardfork, Release> operation_type;

            asset_reserve_evaluator(database &db) : evaluator<asset_reserve_evaluator<Major, Hardfork, Release>, Major,
                    Hardfork, Release>(db) {

            }

            void do_apply(const operation_type &o);

            const asset_dynamic_data_object *asset_dyn_data = nullptr;
            const account_object *from_account = nullptr;
        };

        /**
         *  @brief Evaluator for asset_update_operation
         *  @tparam Major Indicates the major protocol version this operation will be used for
         *  @tparam Hardfork Indicates the hardfork protocol version this operation will be used for
         *  @tparam Release Indicates the release protocol version this operation will be used for
         *
         *  @note In fact business logic protocol versioning scheme is not bounded in any way to business logic chain versioning, but it was decided to make them coincide
         *  @see evaluator
         *  @see asset_update_operation
         */
        template<uint8_t Major, uint8_t Hardfork, uint16_t Release>
        class asset_update_evaluator : public evaluator<asset_update_evaluator<Major, Hardfork, Release>, Major,
                Hardfork, Release> {
        public:
            typedef asset_update_operation<Major, Hardfork, Release> operation_type;

            asset_update_evaluator(database &db) : evaluator<asset_update_evaluator<Major, Hardfork, Release>, Major,
                    Hardfork, Release>(db) {

            }

            void do_apply(const operation_type &o);

            const asset_object *asset_to_update = nullptr;
        };

        /**
         *  @brief Evaluator for asset_update_bitasset_operation
         *  @tparam Major Indicates the major protocol version this operation will be used for
         *  @tparam Hardfork Indicates the hardfork protocol version this operation will be used for
         *  @tparam Release Indicates the release protocol version this operation will be used for
         *
         *  @note In fact business logic protocol versioning scheme is not bounded in any way to business logic chain versioning, but it was decided to make them coincide
         *  @see evaluator
         *  @see asset_update_bitasset_operation
         */
        template<uint8_t Major, uint8_t Hardfork, uint16_t Release>
        class asset_update_bitasset_evaluator : public evaluator<
                asset_update_bitasset_evaluator<Major, Hardfork, Release>, Major, Hardfork, Release> {
        public:
            typedef asset_update_bitasset_operation<Major, Hardfork, Release> operation_type;

            asset_update_bitasset_evaluator(database &db) : evaluator<
                    asset_update_bitasset_evaluator<Major, Hardfork, Release>, Major, Hardfork, Release>(db) {

            }

            void do_apply(const operation_type &o);

            const asset_bitasset_data_object *bitasset_to_update = nullptr;
        };

        /**
         *  @brief Evaluator for asset_update_feed_producers_operation
         *  @tparam Major Indicates the major protocol version this operation will be used for
         *  @tparam Hardfork Indicates the hardfork protocol version this operation will be used for
         *  @tparam Release Indicates the release protocol version this operation will be used for
         *
         *  @note In fact business logic protocol versioning scheme is not bounded in any way to business logic chain versioning, but it was decided to make them coincide
         *  @see evaluator
         *  @see asset_update_feed_producers_operation
         */
        template<uint8_t Major, uint8_t Hardfork, uint16_t Release>
        class asset_update_feed_producers_evaluator : public evaluator<
                asset_update_feed_producers_evaluator<Major, Hardfork, Release>, Major, Hardfork, Release> {
        public:
            typedef asset_update_feed_producers_operation<Major, Hardfork, Release> operation_type;

            asset_update_feed_producers_evaluator(database &db) : evaluator<
                    asset_update_feed_producers_evaluator<Major, Hardfork, Release>, Major, Hardfork, Release>(db) {

            }

            void do_apply(const operation_type &o);

            const asset_bitasset_data_object *bitasset_to_update = nullptr;
        };

        /**
         *  @brief Evaluator for asset_fund_fee_pool_operation
         *  @tparam Major Indicates the major protocol version this operation will be used for
         *  @tparam Hardfork Indicates the hardfork protocol version this operation will be used for
         *  @tparam Release Indicates the release protocol version this operation will be used for
         *
         *  @note In fact business logic protocol versioning scheme is not bounded in any way to business logic chain versioning, but it was decided to make them coincide
         *  @see evaluator
         *  @see asset_fund_fee_pool_operation
         */
        template<uint8_t Major, uint8_t Hardfork, uint16_t Release>
        class asset_fund_fee_pool_evaluator : public evaluator<asset_fund_fee_pool_evaluator<Major, Hardfork, Release>,
                Major, Hardfork, Release> {
        public:
            typedef asset_fund_fee_pool_operation<Major, Hardfork, Release> operation_type;

            asset_fund_fee_pool_evaluator(database &db) : evaluator<
                    asset_fund_fee_pool_evaluator<Major, Hardfork, Release>, Major, Hardfork, Release>(db) {

            }

            void do_apply(const operation_type &op);

            const asset_dynamic_data_object *asset_dyn_data = nullptr;
        };

        /**
         *  @brief Evaluator for asset_global_settle_operation
         *  @tparam Major Indicates the major protocol version this operation will be used for
         *  @tparam Hardfork Indicates the hardfork protocol version this operation will be used for
         *  @tparam Release Indicates the release protocol version this operation will be used for
         *
         *  @note In fact business logic protocol versioning scheme is not bounded in any way to business logic chain versioning, but it was decided to make them coincide
         *  @see evaluator
         *  @see asset_global_settle_operation
         */
        template<uint8_t Major, uint8_t Hardfork, uint16_t Release>
        class asset_global_settle_evaluator : public evaluator<asset_global_settle_evaluator<Major, Hardfork, Release>,
                Major, Hardfork, Release> {
        public:
            typedef asset_global_settle_operation<Major, Hardfork, Release> operation_type;

            asset_global_settle_evaluator(database &db) : evaluator<
                    asset_global_settle_evaluator<Major, Hardfork, Release>, Major, Hardfork, Release>(db) {

            }

            void do_apply(const operation_type &op);

            const asset_object *asset_to_settle = nullptr;
        };

        /**
         *  @brief Evaluator for asset_settle_operation
         *  @tparam Major Indicates the major protocol version this operation will be used for
         *  @tparam Hardfork Indicates the hardfork protocol version this operation will be used for
         *  @tparam Release Indicates the release protocol version this operation will be used for
         *
         *  @note In fact business logic protocol versioning scheme is not bounded in any way to business logic chain versioning, but it was decided to make them coincide
         *  @see evaluator
         *  @see asset_settle_operation
         */
        template<uint8_t Major, uint8_t Hardfork, uint16_t Release>
        class asset_settle_evaluator : public evaluator<asset_settle_evaluator<Major, Hardfork, Release>, Major,
                Hardfork, Release> {
        public:
            typedef asset_settle_operation<Major, Hardfork, Release> operation_type;

            asset_settle_evaluator(database &db) : evaluator<asset_settle_evaluator<Major, Hardfork, Release>, Major,
                    Hardfork, Release>(db) {

            }

            void do_apply(const operation_type &op);

            const asset_object *asset_to_settle = nullptr;
        };

        /**
         *  @brief Evaluator for asset_force_settle_operation
         *  @tparam Major Indicates the major protocol version this operation will be used for
         *  @tparam Hardfork Indicates the hardfork protocol version this operation will be used for
         *  @tparam Release Indicates the release protocol version this operation will be used for
         *
         *  @note In fact business logic protocol versioning scheme is not bounded in any way to business logic chain versioning, but it was decided to make them coincide
         *  @see evaluator
         *  @see asset_force_settle_operation
         */
        template<uint8_t Major, uint8_t Hardfork, uint16_t Release>
        class asset_force_settle_evaluator : public evaluator<asset_force_settle_evaluator<Major, Hardfork, Release>,
                Major, Hardfork, Release> {
        public:
            typedef asset_force_settle_operation<Major, Hardfork, Release> operation_type;

            asset_force_settle_evaluator(database &db) : evaluator<
                    asset_force_settle_evaluator<Major, Hardfork, Release>, Major, Hardfork, Release>(db) {

            }

            void do_apply(const operation_type &op);

            const asset_object *asset_to_settle = nullptr;
        };

        /**
         *  @brief Evaluator for asset_publish_feeds_operation
         *  @tparam Major Indicates the major protocol version this operation will be used for
         *  @tparam Hardfork Indicates the hardfork protocol version this operation will be used for
         *  @tparam Release Indicates the release protocol version this operation will be used for
         *
         *  @note In fact business logic protocol versioning scheme is not bounded in any way to business logic chain versioning, but it was decided to make them coincide
         *  @see evaluator
         *  @see asset_publish_feeds_operation
         */
        template<uint8_t Major, uint8_t Hardfork, uint16_t Release>
        class asset_publish_feeds_evaluator : public evaluator<asset_publish_feeds_evaluator<Major, Hardfork, Release>,
                Major, Hardfork, Release> {
        public:
            typedef asset_publish_feed_operation<Major, Hardfork, Release> operation_type;

            asset_publish_feeds_evaluator(database &db) : evaluator<
                    asset_publish_feeds_evaluator<Major, Hardfork, Release>, Major, Hardfork, Release>(db) {

            }

            void do_apply(const operation_type &o);

            std::map<std::pair<asset_symbol_type, asset_symbol_type>, price_feed<0, 17, 0>> median_feed_values;
        };

        /**
         *  @brief Evaluator for asset_claim_fees_operation
         *  @tparam Major Indicates the major protocol version this operation will be used for
         *  @tparam Hardfork Indicates the hardfork protocol version this operation will be used for
         *  @tparam Release Indicates the release protocol version this operation will be used for
         *
         *  @note In fact business logic protocol versioning scheme is not bounded in any way to business logic chain versioning, but it was decided to make them coincide
         *  @see evaluator
         *  @see asset_claim_fees_operation
         */
        template<uint8_t Major, uint8_t Hardfork, uint16_t Release>
        class asset_claim_fees_evaluator : public evaluator<asset_claim_fees_evaluator<Major, Hardfork, Release>, Major,
                Hardfork, Release> {
        public:
            typedef asset_claim_fees_operation<Major, Hardfork, Release> operation_type;

            asset_claim_fees_evaluator(database &db) : evaluator<asset_claim_fees_evaluator<Major, Hardfork, Release>,
                    Major, Hardfork, Release>(db) {

            }

            void do_apply(const operation_type &o);
        };
    }
} // golos::chain