#ifndef GOLOS_LIMIT_ORDER_CANCEL_EVALUATOR_HPP
#define GOLOS_LIMIT_ORDER_CANCEL_EVALUATOR_HPP

#include "forward.hpp"

namespace steemit {
    namespace chain {

        class limit_order_cancel_evaluator : public evaluator_impl<database_tag, limit_order_cancel_evaluator> {
        public:
            typedef protocol::limit_order_cancel_operation operation_type;

            template<typename Database>
            limit_order_cancel_evaluator(Database &db) : evaluator_impl<database_tag, limit_order_cancel_evaluator>(
                    db) {
            }

            void do_apply(const limit_order_cancel_operation &o);

        };
    }
}
#endif //GOLOS_LIMIT_ORDER_CANCEL_EVALUATOR_HPP
