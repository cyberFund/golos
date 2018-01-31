#pragma once

#include <golos/chain/evaluator.hpp>
#include <golos/chain/database.hpp>
#include <golos/chain/objects/proposal_object.hpp>
#include <golos/chain/transaction_evaluation_state.hpp>

#include <golos/protocol/operations/operations.hpp>
#include <golos/protocol/operations/proposal_operations.hpp>

namespace golos {
    namespace chain {
        /**
         *  @brief Evaluator for proposal_create_operation
         *  @tparam Major Indicates the major protocol version this operation will be used for
         *  @tparam Hardfork Indicates the hardfork protocol version this operation will be used for
         *  @tparam Release Indicates the release protocol version this operation will be used for
         *
         *  @note In fact business logic protocol versioning scheme is not bounded in any way to business logic chain versioning, but it was decided to make them coincide
         *  @see evaluator
         *  @see proposal_create_operation
         */
        template<uint8_t Major, uint8_t Hardfork, uint16_t Release>
        class proposal_create_evaluator : public golos::chain::evaluator<
                proposal_create_evaluator<Major, Hardfork, Release>, Major, Hardfork, Release> {
        public:
            typedef proposal_create_operation<Major, Hardfork, Release> operation_type;

            proposal_create_evaluator(database &db) : golos::chain::evaluator<
                    proposal_create_evaluator<Major, Hardfork, Release>, Major, Hardfork, Release>(db) {
            }

            void do_apply(const operation_type &o);

        protected:
            transaction _proposed_trx;
        };

        /**
         *  @brief Evaluator for proposal_update_operation
         *  @tparam Major Indicates the major protocol version this operation will be used for
         *  @tparam Hardfork Indicates the hardfork protocol version this operation will be used for
         *  @tparam Release Indicates the release protocol version this operation will be used for
         *
         *  @note In fact business logic protocol versioning scheme is not bounded in any way to business logic chain versioning, but it was decided to make them coincide
         *  @see evaluator
         *  @see proposal_update_operation
         */
        template<uint8_t Major, uint8_t Hardfork, uint16_t Release>
        class proposal_update_evaluator : public golos::chain::evaluator<
                proposal_update_evaluator<Major, Hardfork, Release>, Major, Hardfork, Release> {
        public:
            typedef proposal_update_operation<Major, Hardfork, Release> operation_type;

            proposal_update_evaluator(database &db) : golos::chain::evaluator<
                    proposal_update_evaluator<Major, Hardfork, Release>, Major, Hardfork, Release>(db) {
            }

            void do_apply(const operation_type &o);

        protected:
            const proposal_object *_proposal = nullptr;
            bool executed_proposal = false;
            bool proposal_failed = false;
        };

        /**
         *  @brief Evaluator for proposal_delete_operation
         *  @tparam Major Indicates the major protocol version this operation will be used for
         *  @tparam Hardfork Indicates the hardfork protocol version this operation will be used for
         *  @tparam Release Indicates the release protocol version this operation will be used for
         *
         *  @note In fact business logic protocol versioning scheme is not bounded in any way to business logic chain versioning, but it was decided to make them coincide
         *  @see evaluator
         *  @see proposal_delete_operation
         */
        template<uint8_t Major, uint8_t Hardfork, uint16_t Release>
        class proposal_delete_evaluator : public golos::chain::evaluator<
                proposal_delete_evaluator<Major, Hardfork, Release>, Major, Hardfork, Release> {
        public:
            typedef proposal_delete_operation<Major, Hardfork, Release> operation_type;

            proposal_delete_evaluator(database &db) : golos::chain::evaluator<
                    proposal_delete_evaluator<Major, Hardfork, Release>, Major, Hardfork, Release>(db) {

            }

            void do_apply(const operation_type &);

        protected:

            const proposal_object *_proposal = nullptr;
        };
    }
} // golos::chain
