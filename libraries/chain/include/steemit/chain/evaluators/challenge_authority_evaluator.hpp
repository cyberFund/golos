#ifndef GOLOS_CHALLENGE_AUTHORITY_EVALUATOR_HPP
#define GOLOS_CHALLENGE_AUTHORITY_EVALUATOR_HPP

#include "forward.hpp"

namespace steemit {
    namespace chain {

        class challenge_authority_evaluator : public evaluator_impl<database_tag, challenge_authority_evaluator> {
        public:
            typedef protocol::challenge_authority_operation operation_type;

            template<typename DataBase>
            challenge_authority_evaluator(DataBase &db) :evaluator_impl<database_tag, challenge_authority_evaluator>(
                    db) {
            }

            void do_apply(const protocol::challenge_authority_operation &o);
        };
    }
}
#endif //GOLOS_CHALLENGE_AUTHORITY_EVALUATOR_HPP
