#pragma once

#include <steemit/protocol/operations/steem_operations.hpp>

#include <steemit/chain/evaluator.hpp>

namespace steemit {
    namespace chain {
        class witness_update_evaluator
                : public steemit::chain::evaluator<witness_update_evaluator> {
        public:
            typedef protocol::witness_update_operation operation_type;

            witness_update_evaluator(database &db)
                    : steemit::chain::evaluator<witness_update_evaluator>(db) {
            }

            void do_apply(const protocol::witness_update_operation &o);
        };

        class account_witness_vote_evaluator
                : public steemit::chain::evaluator<account_witness_vote_evaluator> {
        public:
            typedef protocol::account_witness_vote_operation operation_type;

            account_witness_vote_evaluator(database &db)
                    : steemit::chain::evaluator<account_witness_vote_evaluator>(db) {
            }

            void do_apply(const protocol::account_witness_vote_operation &o);
        };

        class account_witness_proxy_evaluator
                : public steemit::chain::evaluator<account_witness_proxy_evaluator> {
        public:
            typedef protocol::account_witness_proxy_operation operation_type;

            account_witness_proxy_evaluator(database &db)
                    : steemit::chain::evaluator<account_witness_proxy_evaluator>(db) {
            }

            void do_apply(const protocol::account_witness_proxy_operation &o);
        };

        class withdraw_vesting_evaluator
                : public steemit::chain::evaluator<withdraw_vesting_evaluator> {
        public:
            typedef protocol::withdraw_vesting_operation operation_type;

            withdraw_vesting_evaluator(database &db)
                    : steemit::chain::evaluator<withdraw_vesting_evaluator>(db) {
            }

            void do_apply(const protocol::withdraw_vesting_operation &o);
        };

        class set_withdraw_vesting_route_evaluator
                : public steemit::chain::evaluator<set_withdraw_vesting_route_evaluator> {
        public:
            typedef protocol::set_withdraw_vesting_route_operation operation_type;

            set_withdraw_vesting_route_evaluator(database &db)
                    : steemit::chain::evaluator<set_withdraw_vesting_route_evaluator>(db) {
            }

            void do_apply(const protocol::set_withdraw_vesting_route_operation &o);
        };

        class comment_evaluator
                : public steemit::chain::evaluator<comment_evaluator> {
        public:
            typedef protocol::comment_operation operation_type;

            comment_evaluator(database &db)
                    : steemit::chain::evaluator<comment_evaluator>(db) {
            }

            void do_apply(const protocol::comment_operation &o);
        };

        class comment_options_evaluator
                : public steemit::chain::evaluator<comment_options_evaluator> {
        public:
            typedef protocol::comment_options_operation operation_type;

            comment_options_evaluator(database &db)
                    : steemit::chain::evaluator<comment_options_evaluator>(db) {
            }

            void do_apply(const protocol::comment_options_operation &o);
        };

        class comment_payout_extension_evaluator
                : public steemit::chain::evaluator<comment_payout_extension_evaluator> {
        public:
            typedef protocol::comment_payout_extension_operation operation_type;

            comment_payout_extension_evaluator(database &db)
                    : steemit::chain::evaluator<comment_payout_extension_evaluator>(db) {
            }

            void do_apply(const protocol::comment_payout_extension_operation &o);
        };

        class delete_comment_evaluator
                : public steemit::chain::evaluator<delete_comment_evaluator> {
        public:
            typedef protocol::delete_comment_operation operation_type;

            delete_comment_evaluator(database &db)
                    : steemit::chain::evaluator<delete_comment_evaluator>(db) {
            }

            void do_apply(const protocol::delete_comment_operation &o);
        };

        class vote_evaluator
                : public steemit::chain::evaluator<vote_evaluator> {
        public:
            typedef protocol::vote_operation operation_type;

            vote_evaluator(database &db)
                    : steemit::chain::evaluator<vote_evaluator>(db) {
            }

            void do_apply(const protocol::vote_operation &o);
        };

        class custom_evaluator
                : public steemit::chain::evaluator<custom_evaluator> {
        public:
            typedef protocol::custom_operation operation_type;

            custom_evaluator(database &db)
                    : steemit::chain::evaluator<custom_evaluator>(db) {
            }

            void do_apply(const protocol::custom_operation &o);
        };

        class custom_json_evaluator
                : public steemit::chain::evaluator<custom_json_evaluator> {
        public:
            typedef protocol::custom_json_operation operation_type;

            custom_json_evaluator(database &db)
                    : steemit::chain::evaluator<custom_json_evaluator>(db) {
            }

            void do_apply(const protocol::custom_json_operation &o);
        };

        class custom_binary_evaluator
                : public steemit::chain::evaluator<custom_binary_evaluator> {
        public:
            typedef protocol::custom_binary_operation operation_type;

            custom_binary_evaluator(database &db)
                    : steemit::chain::evaluator<custom_binary_evaluator>(db) {
            }

            void do_apply(const protocol::custom_binary_operation &o);
        };

        class pow_evaluator
                : public steemit::chain::evaluator<pow_evaluator> {
        public:
            typedef protocol::pow_operation operation_type;

            pow_evaluator(database &db)
                    : steemit::chain::evaluator<pow_evaluator>(db) {
            }

            void do_apply(const protocol::pow_operation &o);
        };

        class pow2_evaluator
                : public steemit::chain::evaluator<pow2_evaluator> {
        public:
            typedef protocol::pow2_operation operation_type;

            pow2_evaluator(database &db)
                    : steemit::chain::evaluator<pow2_evaluator>(db) {
            }

            void do_apply(const protocol::pow2_operation &o);
        };

        class feed_publish_evaluator
                : public steemit::chain::evaluator<feed_publish_evaluator> {
        public:
            typedef protocol::feed_publish_operation operation_type;

            feed_publish_evaluator(database &db)
                    : steemit::chain::evaluator<feed_publish_evaluator>(db) {
            }

            void do_apply(const protocol::feed_publish_operation &o);
        };

        class convert_evaluator
                : public steemit::chain::evaluator<convert_evaluator> {
        public:
            typedef protocol::convert_operation operation_type;

            convert_evaluator(database &db)
                    : steemit::chain::evaluator<convert_evaluator>(db) {
            }

            void do_apply(const protocol::convert_operation &o);
        };

        class report_over_production_evaluator
                : public steemit::chain::evaluator<report_over_production_evaluator> {
        public:
            typedef protocol::report_over_production_operation operation_type;

            report_over_production_evaluator(database &db)
                    : steemit::chain::evaluator<report_over_production_evaluator>(db) {
            }

            void do_apply(const protocol::report_over_production_operation &o);
        };

        class escrow_transfer_evaluator
                : public steemit::chain::evaluator<escrow_transfer_evaluator> {
        public:
            typedef protocol::escrow_transfer_operation operation_type;

            escrow_transfer_evaluator(database &db)
                    : steemit::chain::evaluator<escrow_transfer_evaluator>(db) {
            }

            void do_apply(const protocol::escrow_transfer_operation &o);
        };

        class escrow_approve_evaluator
                : public steemit::chain::evaluator<escrow_approve_evaluator> {
        public:
            typedef protocol::escrow_approve_operation operation_type;

            escrow_approve_evaluator(database &db)
                    : steemit::chain::evaluator<escrow_approve_evaluator>(db) {
            }

            void do_apply(const protocol::escrow_approve_operation &o);
        };

        class escrow_dispute_evaluator
                : public steemit::chain::evaluator<escrow_dispute_evaluator> {
        public:
            typedef protocol::escrow_dispute_operation operation_type;

            escrow_dispute_evaluator(database &db)
                    : steemit::chain::evaluator<escrow_dispute_evaluator>(db) {
            }

            void do_apply(const protocol::escrow_dispute_operation &o);
        };

        class escrow_release_evaluator
                : public steemit::chain::evaluator<escrow_release_evaluator> {
        public:
            typedef protocol::escrow_release_operation operation_type;

            escrow_release_evaluator(database &db)
                    : steemit::chain::evaluator<escrow_release_evaluator>(db) {
            }

            void do_apply(const protocol::escrow_release_operation &o);
        };

        class challenge_authority_evaluator
                : public steemit::chain::evaluator<challenge_authority_evaluator> {
        public:
            typedef protocol::challenge_authority_operation operation_type;

            challenge_authority_evaluator(database &db)
                    : steemit::chain::evaluator<challenge_authority_evaluator>(db) {
            }

            void do_apply(const protocol::challenge_authority_operation &o);
        };

        class prove_authority_evaluator
                : public steemit::chain::evaluator<prove_authority_evaluator> {
        public:
            typedef protocol::prove_authority_operation operation_type;

            prove_authority_evaluator(database &db)
                    : steemit::chain::evaluator<prove_authority_evaluator>(db) {
            }

            void do_apply(const protocol::prove_authority_operation &o);
        };

        class request_account_recovery_evaluator
                : public steemit::chain::evaluator<request_account_recovery_evaluator> {
        public:
            typedef protocol::request_account_recovery_operation operation_type;

            request_account_recovery_evaluator(database &db)
                    : steemit::chain::evaluator<request_account_recovery_evaluator>(db) {
            }

            void do_apply(const protocol::request_account_recovery_operation &o);
        };

        class recover_account_evaluator
                : public steemit::chain::evaluator<recover_account_evaluator> {
        public:
            typedef protocol::recover_account_operation operation_type;

            recover_account_evaluator(database &db)
                    : steemit::chain::evaluator<recover_account_evaluator>(db) {
            }

            void do_apply(const protocol::recover_account_operation &o);
        };

        class change_recovery_account_evaluator
                : public steemit::chain::evaluator<change_recovery_account_evaluator> {
        public:
            typedef protocol::change_recovery_account_operation operation_type;

            change_recovery_account_evaluator(database &db)
                    : steemit::chain::evaluator<change_recovery_account_evaluator>(db) {
            }

            void do_apply(const protocol::change_recovery_account_operation &o);
        };

        class decline_voting_rights_evaluator
                : public steemit::chain::evaluator<decline_voting_rights_evaluator> {
        public:
            typedef protocol::decline_voting_rights_operation operation_type;

            decline_voting_rights_evaluator(database &db)
                    : steemit::chain::evaluator<decline_voting_rights_evaluator>(db) {
            }

            void do_apply(const protocol::decline_voting_rights_operation &o);
        };

        class reset_account_evaluator
                : public steemit::chain::evaluator<reset_account_evaluator> {
        public:
            typedef protocol::reset_account_operation operation_type;

            reset_account_evaluator(database &db)
                    : steemit::chain::evaluator<reset_account_evaluator>(db) {
            }

            void do_apply(const protocol::reset_account_operation &o);
        };

        class set_reset_account_evaluator
                : public steemit::chain::evaluator<set_reset_account_evaluator> {
        public:
            typedef protocol::set_reset_account_operation operation_type;

            set_reset_account_evaluator(database &db)
                    : steemit::chain::evaluator<set_reset_account_evaluator>(db) {
            }

            void do_apply(const protocol::set_reset_account_operation &o);
        };

        class delegate_vesting_shares_evaluator
                : public steemit::chain::evaluator<delegate_vesting_shares_evaluator> {
        public:
            typedef protocol::delegate_vesting_shares_operation operation_type;

            delegate_vesting_shares_evaluator(database &db)
                    : steemit::chain::evaluator<delegate_vesting_shares_evaluator>(db) {
            }

            void do_apply(const protocol::delegate_vesting_shares_operation &o);
        };
    }
} // steemit::chain