#ifndef GOLOS_POW2_EVALUATOR_HPP
#define GOLOS_POW2_EVALUATOR_HPP

#include "forward.hpp"

namespace steemit {
    namespace chain {

        class pow2_evaluator : public evaluator_impl<database_tag,pow2_evaluator> {
        public:
            typedef protocol::pow2_operation operation_type;

            template<typename DataBase>
            pow2_evaluator(DataBase &db) : evaluator_impl<database_tag,pow2_evaluator>(db) {
            }

            void do_apply(const protocol::pow2_operation &o);
        };
    }}
#endif //GOLOS_POW2_EVALUATOR_HPP
