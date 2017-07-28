#ifndef GOLOS_ACCOUNT_EVALUATOR_HPP
#define GOLOS_ACCOUNT_EVALUATOR_HPP

#include <steemit/chain/evaluator.hpp>
#include <steemit/protocol/operations/account_operations.hpp>

namespace steemit {
    namespace chain {
        class account_create_evaluator
                : public steemit::chain::evaluator<account_create_evaluator> {
        public:
            typedef protocol::account_create_operation operation_type;

            account_create_evaluator(database &db)
                    : steemit::chain::evaluator<account_create_evaluator>(db) {
            }

            void do_apply(const protocol::account_create_operation &o);
        };

        class account_create_with_delegation_evaluator
                : public steemit::chain::evaluator<account_create_with_delegation_evaluator> {
        public:
            typedef protocol::account_create_with_delegation_operation operation_type;

            account_create_with_delegation_evaluator(database &db)
                    : steemit::chain::evaluator<account_create_with_delegation_evaluator>(db) {
            }

            void do_apply(const protocol::account_create_with_delegation_operation &o);
        };

        class account_update_evaluator
                : public steemit::chain::evaluator<account_update_evaluator> {
        public:
            typedef protocol::account_update_operation operation_type;

            account_update_evaluator(database &db)
                    : steemit::chain::evaluator<account_update_evaluator>(db) {
            }

            void do_apply(const protocol::account_update_operation &o);
        };

        class account_whitelist_evaluator
                : public evaluator<account_whitelist_evaluator> {
        public:
            typedef protocol::account_whitelist_operation operation_type;

            account_whitelist_evaluator(database &db)
                    : steemit::chain::evaluator<account_whitelist_evaluator>(db) {
            }

            void do_apply(const protocol::account_whitelist_operation &o);
        };
    }
}

#endif //GOLOS_ACCOUNT_EVALUATOR_HPP