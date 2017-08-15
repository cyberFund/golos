#ifndef GOLOS_CONVERT_EVALUATOR_HPP
#define GOLOS_CONVERT_EVALUATOR_HPP

#include "forward.hpp"

namespace steemit {
    namespace chain {

        class convert_evaluator : public evaluator_impl<database_set, convert_evaluator> {
        public:
            typedef protocol::convert_operation operation_type;

            template<typename Database>
            convert_evaluator(Database &db) : evaluator_impl<database_set, convert_evaluator>(db) {
            }

            void do_apply(const protocol::convert_operation &o);
        };
    }
}
#endif //GOLOS_CONVERT_EVALUATOR_HPP
