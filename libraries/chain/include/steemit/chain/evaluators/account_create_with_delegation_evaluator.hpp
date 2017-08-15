#ifndef GOLOS_ACCOUNT_CREATE_WITH_DELEGATION_EVALUATOR_HPP
#define GOLOS_ACCOUNT_CREATE_WITH_DELEGATION_EVALUATOR_HPP

#include "forward.hpp"

namespace steemit {
    namespace chain {

        class account_create_with_delegation_evaluator : public evaluator_impl<database_tag,
                account_create_with_delegation_evaluator> {
        public:
            typedef protocol::account_create_with_delegation_operation operation_type;

            template<typename DataBase>
            account_create_with_delegation_evaluator(DataBase &db) : evaluator_impl<database_tag,
                    account_create_with_delegation_evaluator>(db) {
            }

            void do_apply(const account_create_with_delegation_operation &o);
        };
    }
}
#endif //GOLOS_ACCOUNT_CREATE_WITH_DELEGATION_EVALUATOR_HPP
