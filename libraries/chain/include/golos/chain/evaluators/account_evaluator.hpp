#ifndef GOLOS_ACCOUNT_EVALUATOR_HPP
#define GOLOS_ACCOUNT_EVALUATOR_HPP

#include <golos/chain/evaluator.hpp>

#include <golos/protocol/operations/account_operations.hpp>

namespace golos {
    namespace chain {
        /**
         *  @brief Evaluator for account_create_operation
         *  @tparam Major Indicates the major protocol version this operation will be used for
         *  @tparam Hardfork Indicates the hardfork protocol version this operation will be used for
         *  @tparam Release Indicates the release protocol version this operation will be used for
         *
         *  @note In fact business logic protocol versioning scheme is not bounded in any way to business logic chain versioning, but it was decided to make them coincide
         *  @see evaluator
         *  @see account_create_operation
         */
        template<uint8_t Major, uint8_t Hardfork, uint16_t Release>
        class account_create_evaluator : public evaluator<account_create_evaluator<Major, Hardfork, Release>, Major,
                Hardfork, Release> {
        public:
            typedef protocol::account_create_operation<Major, Hardfork, Release> operation_type;

            account_create_evaluator(database &db) : evaluator<account_create_evaluator<Major, Hardfork, Release>,
                    Major, Hardfork, Release>(db) {
            }

            void do_apply(const protocol::account_create_operation<Major, Hardfork, Release> &o);
        };

        /**
         *  @brief Evaluator for account_create_with_delegation_operation
         *  @tparam Major Indicates the major protocol version this operation will be used for
         *  @tparam Hardfork Indicates the hardfork protocol version this operation will be used for
         *  @tparam Release Indicates the release protocol version this operation will be used for
         *
         *  @note In fact business logic protocol versioning scheme is not bounded in any way to business logic chain versioning, but it was decided to make them coincide
         *  @see evaluator
         *  @see account_create_with_delegation_operation
         */
        template<uint8_t Major, uint8_t Hardfork, uint16_t Release>
        class account_create_with_delegation_evaluator : public evaluator<
                account_create_with_delegation_evaluator<Major, Hardfork, Release>, Major, Hardfork, Release> {
        public:
            typedef protocol::account_create_with_delegation_operation<Major, Hardfork, Release> operation_type;

            account_create_with_delegation_evaluator(database &db) : evaluator<
                    account_create_with_delegation_evaluator<Major, Hardfork, Release>, Major, Hardfork, Release>(db) {
            }

            void do_apply(const protocol::account_create_with_delegation_operation<Major, Hardfork, Release> &o);
        };

        /**
         *  @brief Evaluator for account_update_operation
         *  @tparam Major Indicates the major protocol version this operation will be used for
         *  @tparam Hardfork Indicates the hardfork protocol version this operation will be used for
         *  @tparam Release Indicates the release protocol version this operation will be used for
         *
         *  @note In fact business logic protocol versioning scheme is not bounded in any way to business logic chain versioning, but it was decided to make them coincide
         *  @see evaluator
         *  @see account_update_operation
         */
        template<uint8_t Major, uint8_t Hardfork, uint16_t Release>
        class account_update_evaluator : public evaluator<account_update_evaluator<Major, Hardfork, Release>, Major,
                Hardfork, Release> {
        public:
            typedef protocol::account_update_operation<Major, Hardfork, Release> operation_type;

            account_update_evaluator(database &db) : evaluator<account_update_evaluator<Major, Hardfork, Release>,
                    Major, Hardfork, Release>(db) {
            }

            void do_apply(const protocol::account_update_operation<Major, Hardfork, Release> &o);
        };

        /**
         *  @brief Evaluator for account_whitelist_operation
         *  @tparam Major Indicates the major protocol version this operation will be used for
         *  @tparam Hardfork Indicates the hardfork protocol version this operation will be used for
         *  @tparam Release Indicates the release protocol version this operation will be used for
         *
         *  @note In fact business logic protocol versioning scheme is not bounded in any way to business logic chain versioning, but it was decided to make them coincide
         *  @see evaluator
         *  @see account_whitelist_operation
         */
        template<uint8_t Major, uint8_t Hardfork, uint16_t Release>
        class account_whitelist_evaluator : public evaluator<account_whitelist_evaluator<Major, Hardfork, Release>,
                Major, Hardfork, Release> {
        public:
            typedef protocol::account_whitelist_operation<Major, Hardfork, Release> operation_type;

            account_whitelist_evaluator(database &db) : evaluator<account_whitelist_evaluator<Major, Hardfork, Release>,
                    Major, Hardfork, Release>(db) {
            }

            void do_apply(const protocol::account_whitelist_operation<Major, Hardfork, Release> &o);
        };
    }
}

#endif //GOLOS_ACCOUNT_EVALUATOR_HPP