#ifndef GOLOS_DELEGATE_VESTING_SHARES_EVALUATOR_HPP
#define GOLOS_DELEGATE_VESTING_SHARES_EVALUATOR_HPP

#include "forward.hpp"

namespace steemit {
    namespace chain {

        class delegate_vesting_shares_evaluator : public evaluator_impl<database_t,
                delegate_vesting_shares_evaluator> {
        public:
            typedef protocol::delegate_vesting_shares_operation operation_type;

            template<typename Database> delegate_vesting_shares_evaluator(Database &db) : evaluator_impl<database_t,
                    delegate_vesting_shares_evaluator>(db) {
            }

            void do_apply(const protocol::delegate_vesting_shares_operation &op);
        };
    }
}
#endif //GOLOS_DELEGATE_VESTING_SHARES_EVALUATOR_HPP
