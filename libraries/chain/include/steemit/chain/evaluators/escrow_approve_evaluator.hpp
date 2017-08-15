#ifndef GOLOS_ESCROW_APPROVE_EVALUATOR_HPP
#define GOLOS_ESCROW_APPROVE_EVALUATOR_HPP

#include "forward.hpp"

namespace steemit {
    namespace chain {

        class escrow_approve_evaluator : public evaluator_impl<database_t, escrow_approve_evaluator> {
        public:
            typedef protocol::escrow_approve_operation operation_type;

            template<typename Database>
            escrow_approve_evaluator(Database &db) : evaluator_impl<database_t, escrow_approve_evaluator>(db) {
            }

            void do_apply(const protocol::escrow_approve_operation &o);
        };
    }
}
#endif //GOLOS_ESCROW_APPROVE_EVALUATOR_HPP
