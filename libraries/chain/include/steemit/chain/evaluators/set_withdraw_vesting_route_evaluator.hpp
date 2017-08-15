#ifndef GOLOS_SET_WITHDRAW_VESTING_ROUTE_EVALUATOR_HPP
#define GOLOS_SET_WITHDRAW_VESTING_ROUTE_EVALUATOR_HPP

#include "forward.hpp"

namespace steemit {
    namespace chain {

        class set_withdraw_vesting_route_evaluator : public evaluator_impl<database_tag,
                set_withdraw_vesting_route_evaluator> {
        public:
            typedef protocol::set_withdraw_vesting_route_operation operation_type;

            template<typename DataBase>
            set_withdraw_vesting_route_evaluator(DataBase &db) : evaluator_impl<database_tag,
                    set_withdraw_vesting_route_evaluator>(db) {
            }

            void do_apply(const protocol::set_withdraw_vesting_route_operation &o);
        };
    }
}
#endif //GOLOS_SET_WITHDRAW_VESTING_ROUTE_EVALUATOR_HPP
