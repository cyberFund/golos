#ifndef GOLOS_COMMENT_EVALUATOR_HPP
#define GOLOS_COMMENT_EVALUATOR_HPP

#include "forward.hpp"

namespace steemit {
    namespace chain {

        class comment_evaluator : public evaluator_impl<database_tag, comment_evaluator> {
        private :
            struct strcmp_equal {
                bool operator()(const shared_string &a, const string &b) {
                    return a.size() == b.size() || std::strcmp(a.c_str(), b.c_str()) == 0;
                }
            };

        public:
            typedef protocol::comment_operation operation_type;

            template<typename Database>
            comment_evaluator(Database &db) : evaluator_impl<database_tag, comment_evaluator>(db) {
            }

            void do_apply(const protocol::comment_operation &o);
        };
    }
}
#endif //GOLOS_COMMENT_EVALUATOR_HPP
