#pragma once

#include <steemit/chain/evaluator.hpp>
#include <steemit/chain/database.hpp>
#include <steemit/chain/objects/proposal_object.hpp>
#include <steemit/chain/transaction_evaluation_state.hpp>

#include <steemit/protocol/operations/operations.hpp>
#include <steemit/protocol/operations/proposal_operations.hpp>

namespace steemit {
    namespace chain {
        template<uint8_t Major, uint8_t Hardfork, uint16_t Release>
        class proposal_create_evaluator : public steemit::chain::evaluator<
                proposal_create_evaluator<Major, Hardfork, Release>, Major, Hardfork, Release> {
        public:
            typedef proposal_create_operation<Major, Hardfork, Release> operation_type;

            proposal_create_evaluator(database &db) : steemit::chain::evaluator<
                    proposal_create_evaluator<Major, Hardfork, Release>, Major, Hardfork, Release>(db) {
            }

            void do_apply(const operation_type &o);

        protected:
            transaction _proposed_trx;
        };

        template<uint8_t Major, uint8_t Hardfork, uint16_t Release>
        class proposal_update_evaluator : public steemit::chain::evaluator<
                proposal_update_evaluator<Major, Hardfork, Release>, Major, Hardfork, Release> {
        public:
            typedef proposal_update_operation<Major, Hardfork, Release> operation_type;

            proposal_update_evaluator(database &db) : steemit::chain::evaluator<
                    proposal_update_evaluator<Major, Hardfork, Release>, Major, Hardfork, Release>(db) {
            }

            void do_apply(const operation_type &o);

        protected:
            const proposal_object *_proposal = nullptr;
            bool executed_proposal = false;
            bool proposal_failed = false;
        };

        template<uint8_t Major, uint8_t Hardfork, uint16_t Release>
        class proposal_delete_evaluator : public steemit::chain::evaluator<
                proposal_delete_evaluator<Major, Hardfork, Release>, Major, Hardfork, Release> {
        public:
            typedef proposal_delete_operation<Major, Hardfork, Release> operation_type;

            proposal_delete_evaluator(database &db) : steemit::chain::evaluator<
                    proposal_delete_evaluator<Major, Hardfork, Release>, Major, Hardfork, Release>(db) {

            }

            void do_apply(const operation_type &);

        protected:

            const proposal_object *_proposal = nullptr;
        };
    }
} // graphene::chain
