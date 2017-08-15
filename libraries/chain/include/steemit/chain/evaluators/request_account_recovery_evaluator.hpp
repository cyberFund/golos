#ifndef GOLOS_REQUEST_ACCOUNT_RECOVERY_EVALUATOR_HPP
#define GOLOS_REQUEST_ACCOUNT_RECOVERY_EVALUATOR_HPP

#include "forward.hpp"

namespace steemit {
    namespace chain {

        class request_account_recovery_evaluator : public evaluator_impl<database_t,
                request_account_recovery_evaluator> {
        public:
            typedef protocol::request_account_recovery_operation operation_type;

            template<typename Database> request_account_recovery_evaluator(Database &db) : evaluator_impl<database_t,
                    request_account_recovery_evaluator>(db) {
            }

            void do_apply(const protocol::request_account_recovery_operation &o);
        };
    }
}
#endif //GOLOS_REQUEST_ACCOUNT_RECOVERY_EVALUATOR_HPP
