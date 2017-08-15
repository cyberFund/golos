#ifndef GOLOS_ACCOUNT_CREATE_EVALUATOR_HPP
#define GOLOS_ACCOUNT_CREATE_EVALUATOR_HPP

#include "forward.hpp"

namespace steemit {
    namespace chain {


        class account_create_evaluator : public evaluator_impl<database_t, account_create_evaluator> {
        public:
            typedef protocol::account_create_operation operation_type;

            template<typename Database>
            account_create_evaluator(Database &db) : evaluator_impl<database_t, account_create_evaluator>(db) {
            }

            void do_apply(const account_create_operation &o);
        };
    }
}
#endif //GOLOS_ACCOUNT_CREATE_EVALUATOR_HPP
