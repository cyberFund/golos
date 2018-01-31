#pragma once

#include <golos/chain/evaluator.hpp>
#include <golos/chain/objects/account_object.hpp>
#include <golos/chain/objects/asset_object.hpp>
#include <golos/chain/objects/market_object.hpp>

#include <golos/protocol/operations/market_operations.hpp>

#include <golos/protocol/types.hpp>

namespace golos {
    namespace chain {

        /**
         *  @brief Evaluator for convert_operation
         *  @tparam Major Indicates the major protocol version this operation will be used for
         *  @tparam Hardfork Indicates the hardfork protocol version this operation will be used for
         *  @tparam Release Indicates the release protocol version this operation will be used for
         *
         *  @note In fact business logic protocol versioning scheme is not bounded in any way to business logic chain versioning, but it was decided to make them coincide
         *  @see evaluator
         *  @see convert_operation
         */
        template<uint8_t Major, uint8_t Hardfork, uint16_t Release>
        class convert_evaluator : public golos::chain::evaluator<convert_evaluator<Major, Hardfork, Release>, Major,
                Hardfork, Release> {
        public:
            typedef protocol::convert_operation<Major, Hardfork, Release> operation_type;

            convert_evaluator(database &db) : golos::chain::evaluator<convert_evaluator<Major, Hardfork, Release>,
                    Major, Hardfork, Release>(db) {
            }

            void do_apply(const operation_type &o);
        };

        /**
         *  @brief Evaluator for limit_order_create_operation
         *  @tparam Major Indicates the major protocol version this operation will be used for
         *  @tparam Hardfork Indicates the hardfork protocol version this operation will be used for
         *  @tparam Release Indicates the release protocol version this operation will be used for
         *
         *  @note In fact business logic protocol versioning scheme is not bounded in any way to business logic chain versioning, but it was decided to make them coincide
         *  @see evaluator
         *  @see limit_order_create_operation
         */
        template<uint8_t Major, uint8_t Hardfork, uint16_t Release, typename = type_traits::static_range<true>>
        class limit_order_create_evaluator : public evaluator<limit_order_create_evaluator<Major, Hardfork, Release>,
                Major, Hardfork, Release> {
        public:
            typedef protocol::limit_order_create_operation<Major, Hardfork, Release> operation_type;

            limit_order_create_evaluator(database &db) : evaluator<
                    limit_order_create_evaluator<Major, Hardfork, Release>, Major, Hardfork, Release>(db) {

            }
        };

        /**
         *  @brief Evaluator for limit_order_create_operation for the 16-th and lower protocol version
         *  @tparam Major Indicates the major protocol version this operation will be used for
         *  @tparam Hardfork Indicates the hardfork protocol version this operation will be used for
         *  @tparam Release Indicates the release protocol version this operation will be used for
         *
         *  @note In fact business logic protocol versioning scheme is not bounded in any way to business logic chain versioning, but it was decided to make them coincide
         *  @see evaluator
         *  @see limit_order_create_operation
         */
        template<uint8_t Major, uint8_t Hardfork, uint16_t Release>
        class limit_order_create_evaluator<Major, Hardfork, Release, type_traits::static_range<Hardfork <= 16>> : public evaluator<limit_order_create_evaluator<Major, Hardfork, Release>,
                Major, Hardfork, Release> {
        public:
            typedef protocol::limit_order_create_operation<Major, Hardfork, Release> operation_type;

            limit_order_create_evaluator(database &db) : evaluator<
                    limit_order_create_evaluator<Major, Hardfork, Release>, Major, Hardfork, Release>(db) {

            }

            void do_apply(const operation_type &o);
        };

        /**
         *  @brief Evaluator for limit_order_create_operation for the 17-th and higher protocol version
         *  @tparam Major Indicates the major protocol version this operation will be used for
         *  @tparam Hardfork Indicates the hardfork protocol version this operation will be used for
         *  @tparam Release Indicates the release protocol version this operation will be used for
         *
         *  @note In fact business logic protocol versioning scheme is not bounded in any way to business logic chain versioning, but it was decided to make them coincide
         *  @see evaluator
         *  @see limit_order_create_operation
         */
        template<uint8_t Major, uint8_t Hardfork, uint16_t Release>
        class limit_order_create_evaluator<Major, Hardfork, Release, type_traits::static_range<Hardfork >= 17>> : public evaluator<limit_order_create_evaluator<Major, Hardfork, Release>,
                Major, Hardfork, Release> {
        public:
            typedef protocol::limit_order_create_operation<Major, Hardfork, Release> operation_type;

            limit_order_create_evaluator(database &db) : evaluator<
                    limit_order_create_evaluator<Major, Hardfork, Release>, Major, Hardfork, Release>(db) {

            }

            void do_apply(const operation_type &o);

        protected:
            share_type deferred_fee = 0;
            const account_object *seller = nullptr;
            const asset_object *sell_asset = nullptr;
            const asset_object *receive_asset = nullptr;
        };

        /**
         *  @brief Evaluator for limit_order_create2_operation
         *  @tparam Major Indicates the major protocol version this operation will be used for
         *  @tparam Hardfork Indicates the hardfork protocol version this operation will be used for
         *  @tparam Release Indicates the release protocol version this operation will be used for
         *
         *  @note In fact business logic protocol versioning scheme is not bounded in any way to business logic chain versioning, but it was decided to make them coincide
         *  @see evaluator
         *  @see limit_order_create2_operation
         */
        template<uint8_t Major, uint8_t Hardfork, uint16_t Release, typename = type_traits::static_range<true>>
        class limit_order_create2_evaluator : public golos::chain::evaluator<
                limit_order_create2_evaluator<Major, Hardfork, Release>, Major, Hardfork, Release> {
        public:
            typedef protocol::limit_order_create2_operation<Major, Hardfork, Release> operation_type;

            limit_order_create2_evaluator(database &db) : golos::chain::evaluator<
                    limit_order_create2_evaluator<Major, Hardfork, Release>, Major, Hardfork, Release>(db) {
            }
        };

        /**
         *  @brief Evaluator for limit_order_create2_operation for the 16-th and lower replication protocol version
         *  @tparam Major Indicates the major protocol version this operation will be used for
         *  @tparam Hardfork Indicates the hardfork protocol version this operation will be used for
         *  @tparam Release Indicates the release protocol version this operation will be used for
         *
         *  @note In fact business logic protocol versioning scheme is not bounded in any way to business logic chain versioning, but it was decided to make them coincide
         *  @see evaluator
         *  @see limit_order_create2_operation
         */
        template<uint8_t Major, uint8_t Hardfork, uint16_t Release>
        class limit_order_create2_evaluator<Major, Hardfork, Release, type_traits::static_range<Hardfork <= 16>> : public golos::chain::evaluator<
                limit_order_create2_evaluator<Major, Hardfork, Release>, Major, Hardfork, Release> {
        public:
            typedef protocol::limit_order_create2_operation<Major, Hardfork, Release> operation_type;

            limit_order_create2_evaluator(database &db) : golos::chain::evaluator<
                    limit_order_create2_evaluator<Major, Hardfork, Release>, Major, Hardfork, Release>(db) {
            }

            void do_apply(const operation_type &o);
        };

        /**
         *  @brief Evaluator for limit_order_create2_operation for the 17-th and higher replication protocol version
         *  @tparam Major Indicates the major protocol version this operation will be used for
         *  @tparam Hardfork Indicates the hardfork protocol version this operation will be used for
         *  @tparam Release Indicates the release protocol version this operation will be used for
         *
         *  @note In fact business logic protocol versioning scheme is not bounded in any way to business logic chain versioning, but it was decided to make them coincide
         *  @see evaluator
         *  @see limit_order_create2_operation
         */
        template<uint8_t Major, uint8_t Hardfork, uint16_t Release>
        class limit_order_create2_evaluator<Major, Hardfork, Release, type_traits::static_range<Hardfork >= 17>> : public golos::chain::evaluator<
                limit_order_create2_evaluator<Major, Hardfork, Release>, Major, Hardfork, Release> {
        public:
            typedef protocol::limit_order_create2_operation<Major, Hardfork, Release> operation_type;

            limit_order_create2_evaluator(database &db) : golos::chain::evaluator<
                    limit_order_create2_evaluator<Major, Hardfork, Release>, Major, Hardfork, Release>(db) {
            }

            void do_apply(const operation_type &o);

        protected:
            share_type deferred_fee = 0;
            const account_object *seller = nullptr;
            const asset_object *sell_asset = nullptr;
            const asset_object *receive_asset = nullptr;
        };

        /**
         *  @brief Evaluator for limit_order_cancel_operation
         *  @tparam Major Indicates the major protocol version this operation will be used for
         *  @tparam Hardfork Indicates the hardfork protocol version this operation will be used for
         *  @tparam Release Indicates the release protocol version this operation will be used for
         *
         *  @note In fact business logic protocol versioning scheme is not bounded in any way to business logic chain versioning, but it was decided to make them coincide
         *  @see evaluator
         *  @see limit_order_cancel_operation
         */
        template<uint8_t Major, uint8_t Hardfork, uint16_t Release, typename = type_traits::static_range<true>>
        class limit_order_cancel_evaluator : public evaluator<limit_order_cancel_evaluator<Major, Hardfork, Release>,
                Major, Hardfork, Release> {
        public:
            typedef protocol::limit_order_cancel_operation<Major, Hardfork, Release> operation_type;

            limit_order_cancel_evaluator(database &db) : evaluator<
                    limit_order_cancel_evaluator<Major, Hardfork, Release>, Major, Hardfork, Release>(db) {

            }
        };

        /**
         *  @brief Evaluator for limit_order_cancel_operation for the 17-th and higher protocol version
         *  @tparam Major Indicates the major protocol version this operation will be used for
         *  @tparam Hardfork Indicates the hardfork protocol version this operation will be used for
         *  @tparam Release Indicates the release protocol version this operation will be used for
         *
         *  @note In fact business logic protocol versioning scheme is not bounded in any way to business logic chain versioning, but it was decided to make them coincide
         *  @see evaluator
         *  @see limit_order_cancel_operation
         */
        template<uint8_t Major, uint8_t Hardfork, uint16_t Release>
        class limit_order_cancel_evaluator<Major, Hardfork, Release, type_traits::static_range<Hardfork >= 17>> : public evaluator<limit_order_cancel_evaluator<Major, Hardfork, Release>,
                Major, Hardfork, Release> {
        public:
            typedef protocol::limit_order_cancel_operation<Major, Hardfork, Release> operation_type;

            limit_order_cancel_evaluator(database &db) : evaluator<
                    limit_order_cancel_evaluator<Major, Hardfork, Release>, Major, Hardfork, Release>(db) {

            }

            void do_apply(const operation_type &op);

        protected:
            const limit_order_object *_order;
        };

        /**
         *  @brief Evaluator for limit_order_cancel_operation for the 16-th and lower protocol version
         *  @tparam Major Indicates the major protocol version this operation will be used for
         *  @tparam Hardfork Indicates the hardfork protocol version this operation will be used for
         *  @tparam Release Indicates the release protocol version this operation will be used for
         *
         *  @note In fact business logic protocol versioning scheme is not bounded in any way to business logic chain versioning, but it was decided to make them coincide
         *  @see evaluator
         *  @see limit_order_cancel_operation
         */
        template<uint8_t Major, uint8_t Hardfork, uint16_t Release>
        class limit_order_cancel_evaluator<Major, Hardfork, Release, type_traits::static_range<Hardfork <= 16>> : public evaluator<limit_order_cancel_evaluator<Major, Hardfork, Release>,
                Major, Hardfork, Release> {
        public:
            typedef protocol::limit_order_cancel_operation<Major, Hardfork, Release> operation_type;

            limit_order_cancel_evaluator(database &db) : evaluator<
                    limit_order_cancel_evaluator<Major, Hardfork, Release>, Major, Hardfork, Release>(db) {

            }

            void do_apply(const operation_type &op);
        };

        /**
         *  @brief Evaluator for call_order_update_operation
         *  @tparam Major Indicates the major protocol version this operation will be used for
         *  @tparam Hardfork Indicates the hardfork protocol version this operation will be used for
         *  @tparam Release Indicates the release protocol version this operation will be used for
         *
         *  @note In fact business logic protocol versioning scheme is not bounded in any way to business logic chain versioning, but it was decided to make them coincide
         *  @see evaluator
         *  @see call_order_update_operation
         */
        template<uint8_t Major, uint8_t Hardfork, uint16_t Release>
        class call_order_update_evaluator : public evaluator<call_order_update_evaluator<Major, Hardfork, Release>, Major, Hardfork, Release> {
        public:
            typedef protocol::call_order_update_operation<Major, Hardfork, Release> operation_type;

            call_order_update_evaluator(database &db) : evaluator<call_order_update_evaluator<Major, Hardfork, Release>, Major, Hardfork, Release>(db) {

            }

            void do_apply(const operation_type &op);

        protected:
            const asset_object *_debt_asset = nullptr;
            const account_object *_paying_account = nullptr;
            const call_order_object *_order = nullptr;
            const asset_bitasset_data_object *_bitasset_data = nullptr;
        };

        /**
         *  @brief Evaluator for bid_collateral_operation
         *  @tparam Major Indicates the major protocol version this operation will be used for
         *  @tparam Hardfork Indicates the hardfork protocol version this operation will be used for
         *  @tparam Release Indicates the release protocol version this operation will be used for
         *
         *  @note In fact business logic protocol versioning scheme is not bounded in any way to business logic chain versioning, but it was decided to make them coincide
         *  @see evaluator
         *  @see bid_collateral_operation
         */
        template<uint8_t Major, uint8_t Hardfork, uint16_t Release>
        class bid_collateral_evaluator : public evaluator<bid_collateral_evaluator<Major, Hardfork, Release>, Major, Hardfork, Release> {
        public:
            typedef protocol::bid_collateral_operation<Major, Hardfork, Release> operation_type;

            bid_collateral_evaluator(database &db) : evaluator<bid_collateral_evaluator<Major, Hardfork, Release>, Major, Hardfork, Release>(db) {

            }

            void do_apply(const operation_type &o);

            const collateral_bid_object *_bid = nullptr;
        };
    }
} // golos::chain