#ifndef GOLOS_CUSTOM_EVALUATOR_HPP
#define GOLOS_CUSTOM_EVALUATOR_HPP

#include "forward.hpp"

namespace steemit {
    namespace chain {

        class custom_evaluator : public evaluator_impl<database_tag, custom_evaluator> {
        public:
            typedef protocol::custom_operation operation_type;

            template<typename Database>
            custom_evaluator(Database &db) : evaluator_impl<database_tag, custom_evaluator>(db) {
            }

            void do_apply(const protocol::custom_operation &o);
        };
    }
}
#endif //GOLOS_CUSTOM_EVALUATOR_HPP
