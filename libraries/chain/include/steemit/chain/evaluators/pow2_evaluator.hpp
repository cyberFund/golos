#ifndef GOLOS_POW2_EVALUATOR_HPP
#define GOLOS_POW2_EVALUATOR_HPP

#include "forward.hpp"

namespace steemit {
    namespace chain {

        class pow2_evaluator : public evaluator_impl<database_t, pow2_evaluator> {
        public:
            typedef protocol::pow2_operation operation_type;

            template<typename Database>
            pow2_evaluator(Database &db) : evaluator_impl<database_t, pow2_evaluator>(db) {
            }

            void do_apply(const protocol::pow2_operation &o);
        };
    }
}
#endif //GOLOS_POW2_EVALUATOR_HPP
