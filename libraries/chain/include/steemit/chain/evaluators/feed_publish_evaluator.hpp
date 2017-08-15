#ifndef GOLOS_FEED_PUBLISH_EVALUATOR_HPP
#define GOLOS_FEED_PUBLISH_EVALUATOR_HPP

#include "forward.hpp"

namespace steemit {
    namespace chain {

        class feed_publish_evaluator : public evaluator_impl<database_t, feed_publish_evaluator> {
        public:
            typedef protocol::feed_publish_operation operation_type;

            template<typename Database>
            feed_publish_evaluator(Database &db) : evaluator_impl<database_t, feed_publish_evaluator>(db) {
            }

            void do_apply(const protocol::feed_publish_operation &o);
        };
    }
}
#endif //GOLOS_FEED_PUBLISH_EVALUATOR_HPP
