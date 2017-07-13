#ifndef GOLOS_TRANSFER_EVALUATOR_HPP
#define GOLOS_TRANSFER_EVALUATOR_HPP

#include <steemit/protocol/operations/steem_operations.hpp>

#include <steemit/chain/evaluator.hpp>

namespace steemit {
    namespace chain {
        class transfer_evaluator
                : public steemit::chain::evaluator<transfer_evaluator> {
        public:
            typedef protocol::transfer_operation operation_type;

            transfer_evaluator(database &db)
                    : steemit::chain::evaluator<transfer_evaluator>(db) {
            }

            void do_apply(const protocol::transfer_operation &o);
        };

        class transfer_to_vesting_evaluator
                : public steemit::chain::evaluator<transfer_to_vesting_evaluator> {
        public:
            typedef protocol::transfer_to_vesting_operation operation_type;

            transfer_to_vesting_evaluator(database &db)
                    : steemit::chain::evaluator<transfer_to_vesting_evaluator>(db) {
            }

            void do_apply(const protocol::transfer_to_vesting_operation &o);
        };

        class transfer_to_savings_evaluator
                : public steemit::chain::evaluator<transfer_to_savings_evaluator> {
        public:
            typedef protocol::transfer_to_savings_operation operation_type;

            transfer_to_savings_evaluator(database &db)
                    : steemit::chain::evaluator<transfer_to_savings_evaluator>(db) {
            }

            void do_apply(const protocol::transfer_to_savings_operation &o);
        };

        class transfer_from_savings_evaluator
                : public steemit::chain::evaluator<transfer_from_savings_evaluator> {
        public:
            typedef protocol::transfer_from_savings_operation operation_type;

            transfer_from_savings_evaluator(database &db)
                    : steemit::chain::evaluator<transfer_from_savings_evaluator>(db) {
            }

            void do_apply(const protocol::transfer_from_savings_operation &o);
        };

        class cancel_transfer_from_savings_evaluator
                : public steemit::chain::evaluator<cancel_transfer_from_savings_evaluator> {
        public:
            typedef protocol::cancel_transfer_from_savings_operation operation_type;

            cancel_transfer_from_savings_evaluator(database &db)
                    : steemit::chain::evaluator<cancel_transfer_from_savings_evaluator>(db) {
            }

            void do_apply(const protocol::cancel_transfer_from_savings_operation &o);
        };

        class override_transfer_evaluator : public evaluator<override_transfer_evaluator> {
        public:
            typedef protocol::override_transfer_operation operation_type;

            override_transfer_evaluator(database &db)
                    : steemit::chain::evaluator<override_transfer_evaluator>(db) {
            }

            void do_apply(const protocol::override_transfer_operation &o);
        };
    }
}

#endif //GOLOS_TRANSFER_EVALUATOR_HPP
