#ifndef GOLOS_DELEGATE_VESTING_SHARES_EVALUATOR_HPP
#define GOLOS_DELEGATE_VESTING_SHARES_EVALUATOR_HPP

#include "forward.hpp"

namespace steemit {
    namespace chain {

        class delegate_vesting_shares_evaluator : public evaluator_impl<database_tag,
                delegate_vesting_shares_evaluator> {
        public:
            typedef protocol::delegate_vesting_shares_operation operation_type;

            template<typename DataBase> delegate_vesting_shares_evaluator(DataBase &db) : evaluator_impl<database_tag,
                    delegate_vesting_shares_evaluator>(db) {
            }

            void do_apply(const protocol::delegate_vesting_shares_operation &op);
        };
    }
}
#endif //GOLOS_DELEGATE_VESTING_SHARES_EVALUATOR_HPP
