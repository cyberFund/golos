#ifndef GOLOS_CUSTOM_JSON_EVALUATOR_HPP
#define GOLOS_CUSTOM_JSON_EVALUATOR_HPP

#include "forward.hpp"

namespace steemit {
    namespace chain {

        class custom_json_evaluator : public evaluator_impl<database_t, custom_json_evaluator> {
        public:
            typedef protocol::custom_json_operation operation_type;

            template<typename Database>
            custom_json_evaluator(Database &db) : evaluator_impl<database_t, custom_json_evaluator>(db) {
            }

            void do_apply(const protocol::custom_json_operation &o);
        };
    }
}
#endif //GOLOS_CUSTOM_JSON_EVALUATOR_HPP
