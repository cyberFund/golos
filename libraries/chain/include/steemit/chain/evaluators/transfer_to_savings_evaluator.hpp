#ifndef GOLOS_TRANSFER_TO_SAVINGS_EVALUATOR_HPP
#define GOLOS_TRANSFER_TO_SAVINGS_EVALUATOR_HPP

#include "forward.hpp"

namespace steemit {
    namespace chain {

        class transfer_to_savings_evaluator : public evaluator_impl<database_tag,transfer_to_savings_evaluator> {
        public:
            typedef protocol::transfer_to_savings_operation operation_type;

            template<typename DataBase>
            transfer_to_savings_evaluator(DataBase &db) : evaluator_impl<database_tag,transfer_to_savings_evaluator>(db) {
            }

            void do_apply(const transfer_to_savings_operation &op);


        };
    }}
#endif //GOLOS_TRANSFER_TO_SAVINGS_EVALUATOR_HPP
