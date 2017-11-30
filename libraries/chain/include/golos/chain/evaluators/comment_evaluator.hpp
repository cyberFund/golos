#ifndef GOLOS_COMMENT_EVALUATOR_HPP
#define GOLOS_COMMENT_EVALUATOR_HPP

#include <golos/chain/evaluator.hpp>

namespace golos {
    namespace chain {
        template<uint8_t Major, uint8_t Hardfork, uint16_t Release>
        class comment_evaluator : public evaluator<comment_evaluator<Major, Hardfork, Release>, Major, Hardfork,
                Release> {
        public:
            typedef protocol::comment_operation<Major, Hardfork, Release> operation_type;

            comment_evaluator(database &db) : evaluator<comment_evaluator<Major, Hardfork, Release>, Major, Hardfork,
                    Release>(db) {
            }

            void do_apply(const operation_type &o);
        };

        template<uint8_t Major, uint8_t Hardfork, uint16_t Release>
        class comment_options_evaluator : public evaluator<comment_options_evaluator<Major, Hardfork, Release>, Major,
                Hardfork, Release> {
        public:
            typedef protocol::comment_options_operation<Major, Hardfork, Release> operation_type;

            comment_options_evaluator(database &db) : evaluator<comment_options_evaluator<Major, Hardfork, Release>,
                    Major, Hardfork, Release>(db) {
            }

            void do_apply(const operation_type &o);
        };

        template<uint8_t Major, uint8_t Hardfork, uint16_t Release>
        class comment_payout_extension_evaluator : public evaluator<
                comment_payout_extension_evaluator<Major, Hardfork, Release>, Major, Hardfork, Release> {
        public:
            typedef protocol::comment_payout_extension_operation<Major, Hardfork, Release> operation_type;

            comment_payout_extension_evaluator(database &db) : evaluator<
                    comment_payout_extension_evaluator<Major, Hardfork, Release>, Major, Hardfork, Release>(db) {
            }

            void do_apply(const operation_type &o);
        };

        template<uint8_t Major, uint8_t Hardfork, uint16_t Release>
        class delete_comment_evaluator : public evaluator<delete_comment_evaluator<Major, Hardfork, Release>, Major,
                Hardfork, Release> {
        public:
            typedef protocol::delete_comment_operation<Major, Hardfork, Release> operation_type;

            delete_comment_evaluator(database &db) : evaluator<delete_comment_evaluator<Major, Hardfork, Release>,
                    Major, Hardfork, Release>(db) {
            }

            void do_apply(const protocol::delete_comment_operation<Major, Hardfork, Release> &o);
        };
    }
}
#endif //GOLOS_COMMENT_EVALUATOR_HPP