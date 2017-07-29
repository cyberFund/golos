#pragma once

#include <steemit/chain/evaluator.hpp>

#include <steemit/encrypt/custodyutils.hpp>
#include <steemit/protocol/operations/filesystem_operations.hpp>
// return type?

namespace steemit {
    namespace chain {

        static steemit::encrypt::CustodyUtils _custody_utils;

        class set_publishing_manager_evaluator
                : public evaluator<set_publishing_manager_evaluator> {
        public:
            typedef protocol::set_publishing_manager_operation operation_type;

            set_publishing_manager_evaluator(database &db)
                    : evaluator<set_publishing_manager_evaluator>(db) {
            }

            void do_apply(const protocol::set_publishing_manager_operation &o);
        };

        class set_publishing_right_evaluator
                : public evaluator<set_publishing_right_evaluator> {
        public:
            typedef protocol::set_publishing_right_operation operation_type;

            set_publishing_right_evaluator(database &db)
                    : evaluator<set_publishing_right_evaluator>(db) {
            }

            void do_apply(const protocol::set_publishing_right_operation &o);
        };

        class content_submit_evaluator
                : public evaluator<content_submit_evaluator> {
        public:
            typedef protocol::content_submit_operation operation_type;

            content_submit_evaluator(database &db)
                    : evaluator<content_submit_evaluator>(db) {
            }

            bool is_resubmit = false;

            void do_apply(const protocol::content_submit_operation &o);
        };

        class content_cancellation_evaluator
                : public evaluator<content_cancellation_evaluator> {
        public:
            typedef protocol::content_cancellation_operation operation_type;

            void do_apply(const protocol::content_cancellation_operation &o);
        };

        class request_to_buy_evaluator
                : public evaluator<request_to_buy_evaluator> {
        public:
            typedef protocol::request_to_buy_operation operation_type;

            void do_apply(const protocol::request_to_buy_operation &o);

        private:
            bool is_subscriber = false;
        };

        class leave_rating_evaluator
                : public evaluator<leave_rating_evaluator> {
        public:
            typedef protocol::leave_rating_and_comment_operation operation_type;

            void do_apply(const protocol::leave_rating_and_comment_operation &o);
        };

        class ready_to_publish_evaluator
                : public evaluator<ready_to_publish_evaluator> {
        public:
            typedef protocol::ready_to_publish_operation operation_type;

            void do_apply(const protocol::ready_to_publish_operation &o);
        };

        class proof_of_custody_evaluator
                : public evaluator<proof_of_custody_evaluator> {
        private:
        public:
            typedef protocol::proof_of_custody_operation operation_type;

            void do_apply(const protocol::proof_of_custody_operation &o);
        };

        class deliver_keys_evaluator
                : public evaluator<deliver_keys_evaluator> {
        public:
            typedef protocol::deliver_keys_operation operation_type;

            void do_apply(const protocol::deliver_keys_operation &o);
        };

        class return_escrow_submission_evaluator
                : public evaluator<return_escrow_submission_evaluator> {
        public:
            typedef protocol::return_escrow_submission_operation operation_type;

            void do_apply(const protocol::return_escrow_submission_operation &o);
        };

        class return_escrow_buying_evaluator
                : public evaluator<return_escrow_buying_evaluator> {
        public:
            typedef protocol::return_escrow_buying_operation operation_type;

            void do_apply(const protocol::return_escrow_buying_operation &o);
        };

        class report_stats_evaluator
                : public evaluator<report_stats_evaluator> {
        public:
            typedef protocol::report_stats_operation operation_type;

            void do_apply(const protocol::report_stats_operation &o);
        };

        class pay_seeder_evaluator : public evaluator<pay_seeder_evaluator> {
        public:
            typedef protocol::pay_seeder_operation operation_type;

            void do_apply(const protocol::pay_seeder_operation &o);
        };

        class finish_buying_evaluator
                : public evaluator<finish_buying_evaluator> {
        public:
            typedef protocol::finish_buying_operation operation_type;

            void do_apply(const protocol::finish_buying_operation &o);
        };
    }
}