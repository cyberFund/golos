#pragma once

#include <golos/protocol/operations/witness_operations.hpp>

#include <golos/chain/evaluator.hpp>

namespace golos {
    namespace chain {
        /**
         *  @brief Evaluator for withdraw_vesting_operation
         *  @tparam Major Indicates the major protocol version this operation will be used for
         *  @tparam Hardfork Indicates the hardfork protocol version this operation will be used for
         *  @tparam Release Indicates the release protocol version this operation will be used for
         *
         *  @note In fact business logic protocol versioning scheme is not bounded in any way to business logic chain versioning, but it was decided to make them coincide
         *  @see evaluator
         *  @see withdraw_vesting_operation
         */
        template<uint8_t Major, uint8_t Hardfork, uint16_t Release>
        class withdraw_vesting_evaluator : public evaluator<withdraw_vesting_evaluator<Major, Hardfork, Release>, Major,
                Hardfork, Release> {
        public:
            typedef protocol::withdraw_vesting_operation<Major, Hardfork, Release> operation_type;

            withdraw_vesting_evaluator(database &db) : evaluator<withdraw_vesting_evaluator<Major, Hardfork, Release>,
                    Major, Hardfork, Release>(db) {
            }

            void do_apply(const operation_type &o);
        };

        /**
         *  @brief Evaluator for set_withdraw_vesting_route_operation
         *  @tparam Major Indicates the major protocol version this operation will be used for
         *  @tparam Hardfork Indicates the hardfork protocol version this operation will be used for
         *  @tparam Release Indicates the release protocol version this operation will be used for
         *
         *  @note In fact business logic protocol versioning scheme is not bounded in any way to business logic chain versioning, but it was decided to make them coincide
         *  @see evaluator
         *  @see set_withdraw_vesting_route_operation
         */
        template<uint8_t Major, uint8_t Hardfork, uint16_t Release>
        class set_withdraw_vesting_route_evaluator : public evaluator<
                set_withdraw_vesting_route_evaluator<Major, Hardfork, Release>, Major, Hardfork, Release> {
        public:
            typedef protocol::set_withdraw_vesting_route_operation<Major, Hardfork, Release> operation_type;

            set_withdraw_vesting_route_evaluator(database &db) : evaluator<
                    set_withdraw_vesting_route_evaluator<Major, Hardfork, Release>, Major, Hardfork, Release>(db) {
            }

            void do_apply(const operation_type &o);
        };

        /**
         *  @brief Evaluator for vote_operation
         *  @tparam Major Indicates the major protocol version this operation will be used for
         *  @tparam Hardfork Indicates the hardfork protocol version this operation will be used for
         *  @tparam Release Indicates the release protocol version this operation will be used for
         *
         *  @note In fact business logic protocol versioning scheme is not bounded in any way to business logic chain versioning, but it was decided to make them coincide
         *  @see evaluator
         *  @see vote_operation
         */
        template<uint8_t Major, uint8_t Hardfork, uint16_t Release>
        class vote_evaluator : public evaluator<vote_evaluator<Major, Hardfork, Release>, Major, Hardfork, Release> {
        public:
            typedef protocol::vote_operation<Major, Hardfork, Release> operation_type;

            vote_evaluator(database &db) : evaluator<vote_evaluator<Major, Hardfork, Release>, Major, Hardfork,
                    Release>(db) {
            }

            void do_apply(const protocol::vote_operation<Major, Hardfork, Release> &o);
        };

        /**
         *  @brief Evaluator for pow_operation
         *  @tparam Major Indicates the major protocol version this operation will be used for
         *  @tparam Hardfork Indicates the hardfork protocol version this operation will be used for
         *  @tparam Release Indicates the release protocol version this operation will be used for
         *
         *  @note In fact business logic protocol versioning scheme is not bounded in any way to business logic chain versioning, but it was decided to make them coincide
         *  @see evaluator
         *  @see pow_operation
         */
        template<uint8_t Major, uint8_t Hardfork, uint16_t Release, typename = type_traits::static_range<true>>
        class pow_evaluator : public evaluator<pow_evaluator<Major, Hardfork, Release>, Major, Hardfork, Release> {
        };

        /**
         *  @brief Evaluator for pow_operation for the 16-th and lower protocol version
         *  @tparam Major Indicates the major protocol version this operation will be used for
         *  @tparam Hardfork Indicates the hardfork protocol version this operation will be used for
         *  @tparam Release Indicates the release protocol version this operation will be used for
         *
         *  @note In fact business logic protocol versioning scheme is not bounded in any way to business logic chain versioning, but it was decided to make them coincide
         *  @note Disabled, pow2_operation usage is recommended
         *  @see evaluator
         *  @see pow_operation
         */
        template<uint8_t Major, uint8_t Hardfork, uint16_t Release>
        class pow_evaluator<Major, Hardfork, Release, type_traits::static_range<Hardfork <= 16>> : public evaluator<
                pow_evaluator<Major, Hardfork, Release, type_traits::static_range<Hardfork <= 16>>, Major, Hardfork,
                Release> {
        public:
            typedef protocol::pow_operation<Major, Hardfork, Release> operation_type;

            pow_evaluator(database &db) : evaluator<pow_evaluator<Major, Hardfork, Release, type_traits::static_range<Hardfork <= 16>>, Major, Hardfork,
                    Release>(db) {
            }

            void do_apply(const protocol::pow_operation<Major, Hardfork, Release> &o);
        };

        /**
         *  @brief Evaluator for pow_operation for the 17-th and higher protocol version
         *  @tparam Major Indicates the major protocol version this operation will be used for
         *  @tparam Hardfork Indicates the hardfork protocol version this operation will be used for
         *  @tparam Release Indicates the release protocol version this operation will be used for
         *
         *  @note In fact business logic protocol versioning scheme is not bounded in any way to business logic chain versioning, but it was decided to make them coincide
         *  @note Disabled, pow2_operation usage is recommended
         *  @see evaluator
         *  @see pow_operation
         */
        template<uint8_t Major, uint8_t Hardfork, uint16_t Release>
        class pow_evaluator<Major, Hardfork, Release, type_traits::static_range<Hardfork >= 17>> : public evaluator<
                pow_evaluator<Major, Hardfork, Release, type_traits::static_range<Hardfork >= 17>>, Major, Hardfork,
                Release> {
        public:
            typedef protocol::pow_operation<Major, Hardfork, Release> operation_type;

            pow_evaluator(database &db) : evaluator<pow_evaluator<Major, Hardfork, Release, type_traits::static_range<Hardfork >= 17>>, Major, Hardfork,
                    Release>(db) {
            }

            void do_apply(const protocol::pow_operation<Major, Hardfork, Release> &o);
        };

        /**
         *  @brief Evaluator for pow2_operation
         *  @tparam Major Indicates the major protocol version this operation will be used for
         *  @tparam Hardfork Indicates the hardfork protocol version this operation will be used for
         *  @tparam Release Indicates the release protocol version this operation will be used for
         *
         *  @note In fact business logic protocol versioning scheme is not bounded in any way to business logic chain versioning, but it was decided to make them coincide
         *  @see evaluator
         *  @see pow2_operation
         */
        template<uint8_t Major, uint8_t Hardfork, uint16_t Release, typename = type_traits::static_range<true>>
        class pow2_evaluator : public evaluator<pow2_evaluator<Major, Hardfork, Release>, Major, Hardfork, Release> {
        };

        /**
         *  @brief Evaluator for pow2_operation for the 16-th and lower protocol version
         *  @tparam Major Indicates the major protocol version this operation will be used for
         *  @tparam Hardfork Indicates the hardfork protocol version this operation will be used for
         *  @tparam Release Indicates the release protocol version this operation will be used for
         *
         *  @note In fact business logic protocol versioning scheme is not bounded in any way to business logic chain versioning, but it was decided to make them coincide
         *  @see evaluator
         *  @see pow2_operation
         */
        template<uint8_t Major, uint8_t Hardfork, uint16_t Release>
        class pow2_evaluator<Major, Hardfork, Release, type_traits::static_range<Hardfork <= 16>> : public evaluator<
                pow2_evaluator<Major, Hardfork, Release, type_traits::static_range<Hardfork <= 16>>, Major, Hardfork,
                Release> {
        public:
            typedef protocol::pow2_operation<Major, Hardfork, Release> operation_type;

            pow2_evaluator(database &db) : evaluator<pow2_evaluator<Major, Hardfork, Release, type_traits::static_range<Hardfork <= 16>>, Major,
                    Hardfork, Release>(db) {
            }

            void do_apply(const protocol::pow2_operation<Major, Hardfork, Release> &o);
        };

        /**
         *  @brief Evaluator for pow2_operation for the 17-th and higher protocol version
         *  @tparam Major Indicates the major protocol version this operation will be used for
         *  @tparam Hardfork Indicates the hardfork protocol version this operation will be used for
         *  @tparam Release Indicates the release protocol version this operation will be used for
         *
         *  @note In fact business logic protocol versioning scheme is not bounded in any way to business logic chain versioning, but it was decided to make them coincide
         *  @see evaluator
         *  @see pow2_operation
         */
        template<uint8_t Major, uint8_t Hardfork, uint16_t Release>
        class pow2_evaluator<Major, Hardfork, Release, type_traits::static_range<Hardfork >= 17>> : public evaluator<
                pow2_evaluator<Major, Hardfork, Release, type_traits::static_range<Hardfork >= 17>>, Major, Hardfork,
                Release> {
        public:
            typedef protocol::pow2_operation<Major, Hardfork, Release> operation_type;

            pow2_evaluator(database &db) : evaluator<pow2_evaluator<Major, Hardfork, Release, type_traits::static_range<Hardfork >= 17>>, Major, Hardfork,
                    Release>(db) {
            }

            void do_apply(const protocol::pow2_operation<Major, Hardfork, Release> &o);
        };

        /**
         *  @brief Evaluator for feed_publish_operation
         *  @tparam Major Indicates the major protocol version this operation will be used for
         *  @tparam Hardfork Indicates the hardfork protocol version this operation will be used for
         *  @tparam Release Indicates the release protocol version this operation will be used for
         *
         *  @note In fact business logic protocol versioning scheme is not bounded in any way to business logic chain versioning, but it was decided to make them coincide
         *  @see evaluator
         *  @see feed_publish_operation
         */
        template<uint8_t Major, uint8_t Hardfork, uint16_t Release>
        class feed_publish_evaluator : public evaluator<feed_publish_evaluator<Major, Hardfork, Release>, Major,
                Hardfork, Release> {
        public:
            typedef protocol::feed_publish_operation<Major, Hardfork, Release> operation_type;

            feed_publish_evaluator(database &db) : evaluator<feed_publish_evaluator<Major, Hardfork, Release>, Major,
                    Hardfork, Release>(db) {
            }

            void do_apply(const protocol::feed_publish_operation<Major, Hardfork, Release> &o);
        };

        /**
         *  @brief Evaluator for report_over_production_operation
         *  @tparam Major Indicates the major protocol version this operation will be used for
         *  @tparam Hardfork Indicates the hardfork protocol version this operation will be used for
         *  @tparam Release Indicates the release protocol version this operation will be used for
         *
         *  @note In fact business logic protocol versioning scheme is not bounded in any way to business logic chain versioning, but it was decided to make them coincide
         *  @see evaluator
         *  @see report_over_production_operation
         */
        template<uint8_t Major, uint8_t Hardfork, uint16_t Release>
        class report_over_production_evaluator : public evaluator<
                report_over_production_evaluator<Major, Hardfork, Release>, Major, Hardfork, Release> {
        public:
            typedef protocol::report_over_production_operation<Major, Hardfork, Release> operation_type;

            report_over_production_evaluator(database &db) : evaluator<
                    report_over_production_evaluator<Major, Hardfork, Release>, Major, Hardfork, Release>(db) {
            }

            void do_apply(const protocol::report_over_production_operation<Major, Hardfork, Release> &o);
        };

        /**
         *  @brief Evaluator for challenge_authority_operation
         *  @tparam Major Indicates the major protocol version this operation will be used for
         *  @tparam Hardfork Indicates the hardfork protocol version this operation will be used for
         *  @tparam Release Indicates the release protocol version this operation will be used for
         *
         *  @note In fact business logic protocol versioning scheme is not bounded in any way to business logic chain versioning, but it was decided to make them coincide
         *  @see evaluator
         *  @see challenge_authority_operation
         */
        template<uint8_t Major, uint8_t Hardfork, uint16_t Release>
        class challenge_authority_evaluator : public evaluator<challenge_authority_evaluator<Major, Hardfork, Release>,
                Major, Hardfork, Release> {
        public:
            typedef protocol::challenge_authority_operation<Major, Hardfork, Release> operation_type;

            challenge_authority_evaluator(database &db) : evaluator<
                    challenge_authority_evaluator<Major, Hardfork, Release>, Major, Hardfork, Release>(db) {
            }

            void do_apply(const protocol::challenge_authority_operation<Major, Hardfork, Release> &o);
        };

        /**
         *  @brief Evaluator for challenge_authority_evaluator
         *  @tparam Major Indicates the major protocol version this operation will be used for
         *  @tparam Hardfork Indicates the hardfork protocol version this operation will be used for
         *  @tparam Release Indicates the release protocol version this operation will be used for
         *
         *  @note In fact business logic protocol versioning scheme is not bounded in any way to business logic chain versioning, but it was decided to make them coincide
         *  @see evaluator
         *  @see challenge_authority_evaluator
         */
        template<uint8_t Major, uint8_t Hardfork, uint16_t Release>
        class prove_authority_evaluator : public evaluator<prove_authority_evaluator<Major, Hardfork, Release>, Major,
                Hardfork, Release> {
        public:
            typedef protocol::prove_authority_operation<Major, Hardfork, Release> operation_type;

            prove_authority_evaluator(database &db) : evaluator<prove_authority_evaluator<Major, Hardfork, Release>,
                    Major, Hardfork, Release>(db) {
            }

            void do_apply(const protocol::prove_authority_operation<Major, Hardfork, Release> &o);
        };

        /**
         *  @brief Evaluator for request_account_recovery_operation
         *  @tparam Major Indicates the major protocol version this operation will be used for
         *  @tparam Hardfork Indicates the hardfork protocol version this operation will be used for
         *  @tparam Release Indicates the release protocol version this operation will be used for
         *
         *  @note In fact business logic protocol versioning scheme is not bounded in any way to business logic chain versioning, but it was decided to make them coincide
         *  @see evaluator
         *  @see request_account_recovery_operation
         */
        template<uint8_t Major, uint8_t Hardfork, uint16_t Release>
        class request_account_recovery_evaluator : public evaluator<
                request_account_recovery_evaluator<Major, Hardfork, Release>, Major, Hardfork, Release> {
        public:
            typedef protocol::request_account_recovery_operation<Major, Hardfork, Release> operation_type;

            request_account_recovery_evaluator(database &db) : evaluator<
                    request_account_recovery_evaluator<Major, Hardfork, Release>, Major, Hardfork, Release>(db) {
            }

            void do_apply(const protocol::request_account_recovery_operation<Major, Hardfork, Release> &o);
        };

        /**
         *  @brief Evaluator for recover_account_operation
         *  @tparam Major Indicates the major protocol version this operation will be used for
         *  @tparam Hardfork Indicates the hardfork protocol version this operation will be used for
         *  @tparam Release Indicates the release protocol version this operation will be used for
         *
         *  @note In fact business logic protocol versioning scheme is not bounded in any way to business logic chain versioning, but it was decided to make them coincide
         *  @see evaluator
         *  @see recover_account_operation
         */
        template<uint8_t Major, uint8_t Hardfork, uint16_t Release>
        class recover_account_evaluator : public evaluator<recover_account_evaluator<Major, Hardfork, Release>, Major,
                Hardfork, Release> {
        public:
            typedef protocol::recover_account_operation<Major, Hardfork, Release> operation_type;

            recover_account_evaluator(database &db) : evaluator<recover_account_evaluator<Major, Hardfork, Release>,
                    Major, Hardfork, Release>(db) {
            }

            void do_apply(const protocol::recover_account_operation<Major, Hardfork, Release> &o);
        };

        /**
         *  @brief Evaluator for change_recovery_account_operation
         *  @tparam Major Indicates the major protocol version this operation will be used for
         *  @tparam Hardfork Indicates the hardfork protocol version this operation will be used for
         *  @tparam Release Indicates the release protocol version this operation will be used for
         *
         *  @note In fact business logic protocol versioning scheme is not bounded in any way to business logic chain versioning, but it was decided to make them coincide
         *  @see evaluator
         *  @see change_recovery_account_operation
         */
        template<uint8_t Major, uint8_t Hardfork, uint16_t Release>
        class change_recovery_account_evaluator : public evaluator<
                change_recovery_account_evaluator<Major, Hardfork, Release>, Major, Hardfork, Release> {
        public:
            typedef protocol::change_recovery_account_operation<Major, Hardfork, Release> operation_type;

            change_recovery_account_evaluator(database &db) : evaluator<
                    change_recovery_account_evaluator<Major, Hardfork, Release>, Major, Hardfork, Release>(db) {
            }

            void do_apply(const protocol::change_recovery_account_operation<Major, Hardfork, Release> &o);
        };

        /**
         *  @brief Evaluator for decline_voting_rights_operation
         *  @tparam Major Indicates the major protocol version this operation will be used for
         *  @tparam Hardfork Indicates the hardfork protocol version this operation will be used for
         *  @tparam Release Indicates the release protocol version this operation will be used for
         *
         *  @note In fact business logic protocol versioning scheme is not bounded in any way to business logic chain versioning, but it was decided to make them coincide
         *  @see evaluator
         *  @see decline_voting_rights_operation
         */
        template<uint8_t Major, uint8_t Hardfork, uint16_t Release>
        class decline_voting_rights_evaluator : public evaluator<
                decline_voting_rights_evaluator<Major, Hardfork, Release>, Major, Hardfork, Release> {
        public:
            typedef protocol::decline_voting_rights_operation<Major, Hardfork, Release> operation_type;

            decline_voting_rights_evaluator(database &db) : evaluator<
                    decline_voting_rights_evaluator<Major, Hardfork, Release>, Major, Hardfork, Release>(db) {
            }

            void do_apply(const protocol::decline_voting_rights_operation<Major, Hardfork, Release> &o);
        };

        /**
         *  @brief Evaluator for reset_account_operation
         *  @tparam Major Indicates the major protocol version this operation will be used for
         *  @tparam Hardfork Indicates the hardfork protocol version this operation will be used for
         *  @tparam Release Indicates the release protocol version this operation will be used for
         *
         *  @note In fact business logic protocol versioning scheme is not bounded in any way to business logic chain versioning, but it was decided to make them coincide
         *  @see evaluator
         *  @see reset_account_operation
         */
        template<uint8_t Major, uint8_t Hardfork, uint16_t Release>
        class reset_account_evaluator : public evaluator<reset_account_evaluator<Major, Hardfork, Release>, Major,
                Hardfork, Release> {
        public:
            typedef protocol::reset_account_operation<Major, Hardfork, Release> operation_type;

            reset_account_evaluator(database &db) : evaluator<reset_account_evaluator<Major, Hardfork, Release>, Major,
                    Hardfork, Release>(db) {
            }

            void do_apply(const protocol::reset_account_operation<Major, Hardfork, Release> &o);
        };

        /**
         *  @brief Evaluator for set_reset_account_operation
         *  @tparam Major Indicates the major protocol version this operation will be used for
         *  @tparam Hardfork Indicates the hardfork protocol version this operation will be used for
         *  @tparam Release Indicates the release protocol version this operation will be used for
         *
         *  @note In fact business logic protocol versioning scheme is not bounded in any way to business logic chain versioning, but it was decided to make them coincide
         *  @see evaluator
         *  @see set_reset_account_operation
         */
        template<uint8_t Major, uint8_t Hardfork, uint16_t Release>
        class set_reset_account_evaluator : public evaluator<set_reset_account_evaluator<Major, Hardfork, Release>,
                Major, Hardfork, Release> {
        public:
            typedef protocol::set_reset_account_operation<Major, Hardfork, Release> operation_type;

            set_reset_account_evaluator(database &db) : evaluator<set_reset_account_evaluator<Major, Hardfork, Release>,
                    Major, Hardfork, Release>(db) {
            }

            void do_apply(const protocol::set_reset_account_operation<Major, Hardfork, Release> &o);
        };

        /**
         *  @brief Evaluator for delegate_vesting_shares_operation
         *  @tparam Major Indicates the major protocol version this operation will be used for
         *  @tparam Hardfork Indicates the hardfork protocol version this operation will be used for
         *  @tparam Release Indicates the release protocol version this operation will be used for
         *
         *  @note In fact business logic protocol versioning scheme is not bounded in any way to business logic chain versioning, but it was decided to make them coincide
         *  @see evaluator
         *  @see delegate_vesting_shares_operation
         */
        template<uint8_t Major, uint8_t Hardfork, uint16_t Release>
        class delegate_vesting_shares_evaluator : public evaluator<
                delegate_vesting_shares_evaluator<Major, Hardfork, Release>, Major, Hardfork, Release> {
        public:
            typedef protocol::delegate_vesting_shares_operation<Major, Hardfork, Release> operation_type;

            delegate_vesting_shares_evaluator(database &db) : evaluator<
                    delegate_vesting_shares_evaluator<Major, Hardfork, Release>, Major, Hardfork, Release>(db) {
            }

            void do_apply(const protocol::delegate_vesting_shares_operation<Major, Hardfork, Release> &o);
        };
    }
} // golos::chain