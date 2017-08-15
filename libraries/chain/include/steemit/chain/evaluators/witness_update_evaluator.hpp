#ifndef GOLOS_WITNESS_UPDATE_EVALUATOR_HPP
#define GOLOS_WITNESS_UPDATE_EVALUATOR_HPP

#include "forward.hpp"

namespace steemit {
    namespace chain {

        class witness_update_evaluator : public evaluator_impl<database_tag, witness_update_evaluator> {
        public:
            typedef protocol::witness_update_operation operation_type;

            template<typename DataBase>
            witness_update_evaluator(DataBase &db) : evaluator_impl<database_tag, witness_update_evaluator>(db) {
            }

            void do_apply(const protocol::witness_update_operation &o);
        };
    }
}
#endif //GOLOS_WITNESS_UPDATE_EVALUATOR_HPP
