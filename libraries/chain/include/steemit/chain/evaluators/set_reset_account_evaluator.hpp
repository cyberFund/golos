#ifndef GOLOS_SET_RESET_ACCOUNT_EVALUATOR_HPP
#define GOLOS_SET_RESET_ACCOUNT_EVALUATOR_HPP

#include "forward.hpp"

namespace steemit {
    namespace chain {

        class set_reset_account_evaluator : public evaluator_impl<database_set, set_reset_account_evaluator> {
        public:
            typedef protocol::set_reset_account_operation operation_type;

            template<typename Database>
            set_reset_account_evaluator(Database &db) : evaluator_impl<database_set, set_reset_account_evaluator>(db) {
            }

            void do_apply(const protocol::set_reset_account_operation &op);
        };
    }
}
#endif //GOLOS_SET_RESET_ACCOUNT_EVALUATOR_HPP
