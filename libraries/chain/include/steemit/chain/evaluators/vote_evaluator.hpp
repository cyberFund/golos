#ifndef GOLOS_VOTE_EVALUATOR_HPP
#define GOLOS_VOTE_EVALUATOR_HPP

#include "forward.hpp"

namespace steemit {
    namespace chain {

        class vote_evaluator : public evaluator_impl<database_tag, vote_evaluator> {
        public:
            typedef protocol::vote_operation operation_type;

            template<typename Database>
            vote_evaluator(Database &db) : evaluator_impl<database_tag, vote_evaluator>(db) {
            }

            void do_apply(const protocol::vote_operation &o);
        };
    }
}
#endif //GOLOS_VOTE_EVALUATOR_HPP
