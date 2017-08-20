#ifndef GOLOS_LIMIT_ORDER_CREATE2_EVALUATOR_HPP
#define GOLOS_LIMIT_ORDER_CREATE2_EVALUATOR_HPP

#include "forward.hpp"

namespace steemit {
    namespace chain {

        class limit_order_create2_evaluator : public evaluator_impl<database_t, limit_order_create2_evaluator> {
        public:
            typedef protocol::limit_order_create2_operation operation_type;

            template<typename Database>
            limit_order_create2_evaluator(Database &db) : evaluator_impl<database_t, limit_order_create2_evaluator>(
                    db) {
            }

            void do_apply(const limit_order_create2_operation &o);

        };
    }
}
#endif //GOLOS_LIMIT_ORDER_CREATE2_EVALUATOR_HPP
