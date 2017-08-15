#ifndef GOLOS_PROVE_AUTHORITY_EVALUATOR_HPP
#define GOLOS_PROVE_AUTHORITY_EVALUATOR_HPP

#include "forward.hpp"

namespace steemit {
    namespace chain {

        class prove_authority_evaluator : public evaluator_impl<database_t, prove_authority_evaluator> {
        public:
            typedef protocol::prove_authority_operation operation_type;

            template<typename Database>
            prove_authority_evaluator(Database &db) : evaluator_impl<database_t, prove_authority_evaluator>(db) {
            }

            void do_apply(const protocol::prove_authority_operation &o);
        };
    }
}
#endif //GOLOS_PROVE_AUTHORITY_EVALUATOR_HPP
