#ifndef GOLOS_COMMENT_EVALUATOR_HPP
#define GOLOS_COMMENT_EVALUATOR_HPP

#include <golos/chain/evaluator.hpp>

namespace golos {
    namespace chain {
        /**
         *  @brief Evaluator for comment_operation
         *  @tparam Major Indicates the major protocol version this operation will be used for
         *  @tparam Hardfork Indicates the hardfork protocol version this operation will be used for
         *  @tparam Release Indicates the release protocol version this operation will be used for
         *
         *  @note In fact business logic protocol versioning scheme is not bounded in any way to business logic chain versioning, but it was decided to make them coincide
         *  @see evaluator
         *  @see comment_operation
         */
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

        /**
         *  @brief Evaluator for comment_options_operation
         *  @tparam Major Indicates the major protocol version this operation will be used for
         *  @tparam Hardfork Indicates the hardfork protocol version this operation will be used for
         *  @tparam Release Indicates the release protocol version this operation will be used for
         *
         *  @note In fact business logic protocol versioning scheme is not bounded in any way to business logic chain versioning, but it was decided to make them coincide
         *  @see evaluator
         *  @see comment_options_operation
         */
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

        /**
         *  @brief Evaluator for delete_comment_operation
         *  @tparam Major Indicates the major protocol version this operation will be used for
         *  @tparam Hardfork Indicates the hardfork protocol version this operation will be used for
         *  @tparam Release Indicates the release protocol version this operation will be used for
         *
         *  @note In fact business logic protocol versioning scheme is not bounded in any way to business logic chain versioning, but it was decided to make them coincide
         *  @see evaluator
         *  @see delete_comment_operation
         */
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