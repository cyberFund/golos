#ifndef GOLOS_ESCROW_DISPUTE_EVALUATOR_HPP
#define GOLOS_ESCROW_DISPUTE_EVALUATOR_HPP

#include "forward.hpp"

namespace steemit {
    namespace chain {

        class escrow_dispute_evaluator : public evaluator_impl<database_tag,escrow_dispute_evaluator> {
        public:
            typedef protocol::escrow_dispute_operation operation_type;

            template<typename DataBase>
            escrow_dispute_evaluator(DataBase &db) : evaluator_impl<database_tag,escrow_dispute_evaluator>(db) {
            }

            void do_apply(const protocol::escrow_dispute_operation &o);
        };
    }}
#endif //GOLOS_ESCROW_DISPUTE_EVALUATOR_HPP
