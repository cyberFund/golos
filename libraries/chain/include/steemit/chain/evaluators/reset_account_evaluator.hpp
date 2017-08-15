#ifndef GOLOS_RESET_ACCOUNT_EVALUATOR_HPP
#define GOLOS_RESET_ACCOUNT_EVALUATOR_HPP

#include "forward.hpp"

namespace steemit {
    namespace chain {

        class reset_account_evaluator : public evaluator_impl<database_tag, reset_account_evaluator> {
        public:
            typedef protocol::reset_account_operation operation_type;

            template<typename DataBase>
            reset_account_evaluator(DataBase &db) : evaluator_impl<database_tag, reset_account_evaluator>(db) {
            }

            void do_apply(const protocol::reset_account_operation &op);
        };
    }
}
#endif //GOLOS_RESET_ACCOUNT_EVALUATOR_HPP
