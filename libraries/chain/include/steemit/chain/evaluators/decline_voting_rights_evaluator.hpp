#ifndef GOLOS_DECLINE_VOTING_RIGHTS_EVALUATOR_HPP
#define GOLOS_DECLINE_VOTING_RIGHTS_EVALUATOR_HPP

#include "forward.hpp"

namespace steemit {
    namespace chain {

        class decline_voting_rights_evaluator : public evaluator_impl<database_tag, decline_voting_rights_evaluator> {
        public:
            typedef protocol::decline_voting_rights_operation operation_type;

            template<typename DataBase> decline_voting_rights_evaluator(DataBase &db) : evaluator_impl<database_tag,
                    decline_voting_rights_evaluator>(db) {
            }

            void do_apply(const protocol::decline_voting_rights_operation &o);
        };
    }
}
#endif //GOLOS_DECLINE_VOTING_RIGHTS_EVALUATOR_HPP
