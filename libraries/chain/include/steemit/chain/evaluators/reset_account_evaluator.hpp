#ifndef GOLOS_RESET_ACCOUNT_EVALUATOR_HPP
#define GOLOS_RESET_ACCOUNT_EVALUATOR_HPP

#include "forward.hpp"

namespace steemit {
    namespace chain {

        class reset_account_evaluator : public evaluator_impl<database_t, reset_account_evaluator> {
        public:
            typedef protocol::reset_account_operation operation_type;

            template<typename Database>
            reset_account_evaluator(Database &db) : evaluator_impl<database_t, reset_account_evaluator>(db) {
            }

            void do_apply(const protocol::reset_account_operation &op);
        };
    }
}
#endif //GOLOS_RESET_ACCOUNT_EVALUATOR_HPP
