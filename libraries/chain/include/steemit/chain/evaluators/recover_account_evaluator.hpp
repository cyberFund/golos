#ifndef GOLOS_RECOVER_ACCOUNT_EVALUATOR_HPP
#define GOLOS_RECOVER_ACCOUNT_EVALUATOR_HPP

#include "forward.hpp"

namespace steemit {
    namespace chain {

        class recover_account_evaluator : public evaluator_impl<database_set, recover_account_evaluator> {
        public:
            typedef protocol::recover_account_operation operation_type;

            template<typename Database>
            recover_account_evaluator(Database &db) : evaluator_impl<database_set, recover_account_evaluator>(db) {
            }

            void do_apply(const protocol::recover_account_operation &o);
        };
    }
}
#endif //GOLOS_RECOVER_ACCOUNT_EVALUATOR_HPP
