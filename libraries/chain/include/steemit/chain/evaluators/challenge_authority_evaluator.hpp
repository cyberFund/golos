#ifndef GOLOS_CHALLENGE_AUTHORITY_EVALUATOR_HPP
#define GOLOS_CHALLENGE_AUTHORITY_EVALUATOR_HPP

#include "forward.hpp"

namespace steemit {
    namespace chain {

        class challenge_authority_evaluator : public evaluator_impl<database_set, challenge_authority_evaluator> {
        public:
            typedef protocol::challenge_authority_operation operation_type;

            template<typename Database>
            challenge_authority_evaluator(Database &db) :evaluator_impl<database_set, challenge_authority_evaluator>(
                    db) {
            }

            void do_apply(const protocol::challenge_authority_operation &o);
        };
    }
}
#endif //GOLOS_CHALLENGE_AUTHORITY_EVALUATOR_HPP
