#ifndef GOLOS_CHANGE_RECOVERY_ACCOUNT_EVALUATOR_HPP
#define GOLOS_CHANGE_RECOVERY_ACCOUNT_EVALUATOR_HPP

#include "forward.hpp"

namespace steemit {
    namespace chain {

        class change_recovery_account_evaluator : public evaluator_impl<database_tag,
                change_recovery_account_evaluator> {
        public:
            typedef protocol::change_recovery_account_operation operation_type;

            template<typename DataBase> change_recovery_account_evaluator(DataBase &db) : evaluator_impl<database_tag,
                    change_recovery_account_evaluator>(db) {
            }

            void do_apply(const protocol::change_recovery_account_operation &o);
        };
    }
}

#endif //GOLOS_CHANGE_RECOVERY_ACCOUNT_EVALUATOR_HPP
