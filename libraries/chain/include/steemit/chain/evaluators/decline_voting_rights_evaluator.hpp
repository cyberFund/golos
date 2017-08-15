#ifndef GOLOS_DECLINE_VOTING_RIGHTS_EVALUATOR_HPP
#define GOLOS_DECLINE_VOTING_RIGHTS_EVALUATOR_HPP

#include "forward.hpp"

namespace steemit {
    namespace chain {

        class decline_voting_rights_evaluator : public evaluator_impl<database_t, decline_voting_rights_evaluator> {
        public:
            typedef protocol::decline_voting_rights_operation operation_type;

            template<typename Database> decline_voting_rights_evaluator(Database &db) : evaluator_impl<database_t,
                    decline_voting_rights_evaluator>(db) {
            }

            void do_apply(const protocol::decline_voting_rights_operation &o);
        };
    }
}
#endif //GOLOS_DECLINE_VOTING_RIGHTS_EVALUATOR_HPP
