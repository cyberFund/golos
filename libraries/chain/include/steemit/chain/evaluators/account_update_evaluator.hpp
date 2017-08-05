#ifndef GOLOS_ACCOUNT_UPDATE_EVALUATOR_HPP
#define GOLOS_ACCOUNT_UPDATE_EVALUATOR_HPP

#include "forward.hpp"

namespace steemit {
    namespace chain {

        class account_update_evaluator : public evaluator_impl<database_tag,account_update_evaluator> {
        public:
            typedef protocol::account_update_operation operation_type;

            template<typename DataBase>
            account_update_evaluator(DataBase &db) : evaluator_impl<database_tag,account_update_evaluator>(db) {
            }

            void do_apply(const account_update_operation &o);
        };
    }}
#endif //GOLOS_ACCOUNT_UPDATE_EVALUATOR_HPP
