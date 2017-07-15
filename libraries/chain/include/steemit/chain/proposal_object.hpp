#pragma once

#include <steemit/chain/transaction_evaluation_state.hpp>

#include <steemit/protocol/transaction.hpp>

namespace steemit {
    namespace chain {
        /**
         *  @brief tracks the approval of a partially approved transaction
         *  @ingroup object
         *  @ingroup protocol
         */
        class proposal_object : public object<proposal_object_type, proposal_object> {
        public:
            template<typename Constructor, typename Allocator>
            proposal_object(Constructor &&c, allocator<Allocator> a) {
                c(*this);
            };

            id_type id;

            time_point_sec expiration_time;
            optional<time_point_sec> review_period_time;
            transaction proposed_transaction;
            flat_set<account_name_type> required_active_approvals;
            flat_set<account_name_type> available_active_approvals;
            flat_set<account_name_type> required_owner_approvals;
            flat_set<account_name_type> available_owner_approvals;
            flat_set<account_name_type> required_posting_approvals;
            flat_set<account_name_type> available_posting_approvals;
            flat_set<public_key_type> available_key_approvals;

            bool is_authorized_to_execute(database &db) const;
        };

        /**
         *  @brief tracks all of the proposal objects that requrie approval of
         *  an individual account.
         *
         *  @ingroup object
         *  @ingroup protocol
         *
         *  This is a secondary index on the proposal_index
         *
         *  @note the set of required approvals is constant
         */
        class required_approval_index : public chainbase::secondary_index<proposal_object> {
        public:
            virtual void object_inserted(const index_type &obj) override;

            virtual void object_removed(const index_type &obj) override;

            virtual void about_to_modify(const index_type &before) override {
            };

            virtual void object_modified(const index_type &after) override {
            };

            void remove(account_name_type a, proposal_object::id_type p);

            map<account_name_type, set<proposal_object::id_type>> account_to_proposals;
        };

        struct by_expiration;

        typedef boost::multi_index_container<
                proposal_object,
                indexed_by<
                        ordered_unique<tag<
                                by_id>, member<proposal_object, proposal_object::id_type, &proposal_object::id>>,
                        ordered_non_unique<tag<by_expiration>, member<proposal_object, time_point_sec, &proposal_object::expiration_time>>
                >, allocator<proposal_object>
        > proposal_index;
    }
}

FC_REFLECT(steemit::chain::proposal_object, (expiration_time)(review_period_time)(proposed_transaction)(required_active_approvals)(available_active_approvals)(required_owner_approvals)(available_owner_approvals)(available_key_approvals));
CHAINBASE_SET_INDEX_TYPE(steemit::chain::proposal_object, steemit::chain::proposal_index);