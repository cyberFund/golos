#ifndef GOLOS_TRANSFER_EVALUATOR_HPP
#define GOLOS_TRANSFER_EVALUATOR_HPP

#include "forward.hpp"

namespace steemit {
    namespace chain {

        class transfer_evaluator : public evaluator_impl<database_tag, transfer_evaluator> {
        public:
            typedef protocol::transfer_operation operation_type;

            template<typename DataBase>
            transfer_evaluator(DataBase &db) : evaluator_impl<database_tag, transfer_evaluator>(db) {
            }

            void do_apply(const transfer_operation &o);


        };
    }
}
#endif //GOLOS_TRANSFER_EVALUATOR_HPP
