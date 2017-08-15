#ifndef GOLOS_TRANSFER_FROM_SAVINGS_EVALUATOR_HPP
#define GOLOS_TRANSFER_FROM_SAVINGS_EVALUATOR_HPP

#include "forward.hpp"

namespace steemit {
    namespace chain {

        class transfer_from_savings_evaluator : public evaluator_impl<database_tag, transfer_from_savings_evaluator> {
        public:
            typedef protocol::transfer_from_savings_operation operation_type;

            template<typename DataBase> transfer_from_savings_evaluator(DataBase &db) : evaluator_impl<database_tag,
                    transfer_from_savings_evaluator>(db) {
            }

            void do_apply(const transfer_from_savings_operation &op);


        };
    }
}
#endif //GOLOS_TRANSFER_FROM_SAVINGS_EVALUATOR_HPP
