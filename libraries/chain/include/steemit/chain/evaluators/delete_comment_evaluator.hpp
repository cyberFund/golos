#ifndef GOLOS_DELETE_COMMENT_EVALUATOR_HPP
#define GOLOS_DELETE_COMMENT_EVALUATOR_HPP

#include "forward.hpp"

namespace steemit {
    namespace chain {

        class delete_comment_evaluator : public evaluator_impl<database_tag, delete_comment_evaluator> {
        public:
            typedef protocol::delete_comment_operation operation_type;

            template<typename DataBase>
            delete_comment_evaluator(DataBase &db) : evaluator_impl<database_tag, delete_comment_evaluator>(db) {
            }

            /**
             *  Because net_rshares is 0 there is no need to update any pending payout calculations or parent posts.
             */
            void do_apply(const protocol::delete_comment_operation &o);
        };
    }
}
#endif //GOLOS_DELETE_COMMENT_EVALUATOR_HPP
