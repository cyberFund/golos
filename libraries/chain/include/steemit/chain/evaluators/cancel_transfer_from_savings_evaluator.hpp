#ifndef GOLOS_CANCEL_TRANSFER_FROM_SAVINGS_EVALUATOR_HPP
#define GOLOS_CANCEL_TRANSFER_FROM_SAVINGS_EVALUATOR_HPP

#include "forward.hpp"

namespace steemit {
    namespace chain {

        class cancel_transfer_from_savings_evaluator : public evaluator_impl<database_tag,cancel_transfer_from_savings_evaluator> {
        public:
            typedef protocol::cancel_transfer_from_savings_operation operation_type;

            template<typename DataBase>
            cancel_transfer_from_savings_evaluator(DataBase &db) :evaluator_impl<database_tag,cancel_transfer_from_savings_evaluator>(db) {
            }

            void do_apply(const cancel_transfer_from_savings_operation &op);


        };
    }}
#endif //GOLOS_CANCEL_TRANSFER_FROM_SAVINGS_EVALUATOR_HPP
