#ifndef GOLOS_CHANGE_RECOVERY_ACCOUNT_EVALUATOR_HPP
#define GOLOS_CHANGE_RECOVERY_ACCOUNT_EVALUATOR_HPP

#include "forward.hpp"

namespace steemit {
    namespace chain {

        class change_recovery_account_evaluator : public evaluator_impl<database_set,
                change_recovery_account_evaluator> {
        public:
            typedef protocol::change_recovery_account_operation operation_type;

            template<typename Database> change_recovery_account_evaluator(Database &db) : evaluator_impl<database_set,
                    change_recovery_account_evaluator>(db) {
            }

            void do_apply(const protocol::change_recovery_account_operation &o);
        };
    }
}

#endif //GOLOS_CHANGE_RECOVERY_ACCOUNT_EVALUATOR_HPP
