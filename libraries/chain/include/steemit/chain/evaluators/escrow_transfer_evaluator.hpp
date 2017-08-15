#ifndef GOLOS_ESCROW_TRANSFER_EVALUATOR_HPP
#define GOLOS_ESCROW_TRANSFER_EVALUATOR_HPP

#include "forward.hpp"

namespace steemit {
    namespace chain {

        class escrow_transfer_evaluator : public evaluator_impl<database_tag, escrow_transfer_evaluator> {
        public:
            typedef protocol::escrow_transfer_operation operation_type;

            template<typename DataBase>
            escrow_transfer_evaluator(DataBase &db) : evaluator_impl<database_tag, escrow_transfer_evaluator>(db) {
            }

            void do_apply(const protocol::escrow_transfer_operation &o);
        };
    }
}
#endif //GOLOS_ESCROW_TRANSFER_EVALUATOR_HPP
