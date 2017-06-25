#pragma once

#include <steemit/chain/evaluator.hpp>
#include <steemit/chain/committee_member_object.hpp>

#include <steemit/protocol/committee_member_operations.hpp>

namespace steemit {
    namespace chain {

        class committee_member_create_evaluator
                : public steemit::chain::evaluator_impl<committee_member_create_evaluator> {
        public:
            typedef protocol::committee_member_create_operation operation_type;

            committee_member_create_evaluator(database &db)
                    : steemit::chain::evaluator_impl<committee_member_create_evaluator>(db) {
            }

            void do_apply(const protocol::committee_member_create_operation &o);
        };

        class committee_member_update_evaluator
                : public steemit::chain::evaluator_impl<committee_member_update_evaluator> {
        public:
            typedef protocol::committee_member_update_operation operation_type;

            committee_member_update_evaluator(database &db)
                    : steemit::chain::evaluator_impl<committee_member_update_evaluator>(db) {
            }

            void do_apply(const protocol::committee_member_update_operation &o);
        };

        class committee_member_update_global_parameters_evaluator
                : public steemit::chain::evaluator_impl<committee_member_update_global_parameters_evaluator> {
        public:
            typedef protocol::committee_member_update_global_parameters_operation operation_type;

            committee_member_update_global_parameters_evaluator(database &db)
                    : steemit::chain::evaluator_impl<committee_member_update_global_parameters_evaluator>(db) {
            }

            void do_apply(const protocol::committee_member_update_global_parameters_operation &o);
        };
    }
}
