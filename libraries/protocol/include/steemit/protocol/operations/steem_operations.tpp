namespace steemit {
    namespace protocol {
        extern template struct challenge_authority_operation<0, 16, 0>;
        extern template struct challenge_authority_operation<0, 17, 0>;

        extern template struct prove_authority_operation<0, 16, 0>;
        extern template struct prove_authority_operation<0, 17, 0>;

        extern template struct vote_operation<0, 16, 0>;
        extern template struct vote_operation<0, 17, 0>;

        extern template struct withdraw_vesting_operation<0, 16, 0>;
        extern template struct withdraw_vesting_operation<0, 17, 0>;

        extern template struct set_withdraw_vesting_route_operation<0, 16, 0>;
        extern template struct set_withdraw_vesting_route_operation<0, 17, 0>;

        extern template struct witness_update_operation<0, 16, 0>;
        extern template struct witness_update_operation<0, 17, 0>;

        extern template struct account_witness_vote_operation<0, 16, 0>;
        extern template struct account_witness_vote_operation<0, 17, 0>;

        extern template struct account_witness_proxy_operation<0, 16, 0>;
        extern template struct account_witness_proxy_operation<0, 17, 0>;

        extern template struct feed_publish_operation<0, 16, 0>;
        extern template struct feed_publish_operation<0, 17, 0>;

        extern template struct pow_operation<0, 16, 0>;
        extern template struct pow_operation<0, 17, 0>;

        extern template struct pow2_operation<0, 16, 0>;
        extern template struct pow2_operation<0, 17, 0>;

        extern template struct report_over_production_operation<0, 16, 0>;
        extern template struct report_over_production_operation<0, 17, 0>;

        extern template struct request_account_recovery_operation<0, 16, 0>;
        extern template struct request_account_recovery_operation<0, 17, 0>;

        extern template struct recover_account_operation<0, 16, 0>;
        extern template struct recover_account_operation<0, 17, 0>;

        extern template struct reset_account_operation<0, 16, 0>;
        extern template struct reset_account_operation<0, 17, 0>;

        extern template struct set_reset_account_operation<0, 16, 0>;
        extern template struct set_reset_account_operation<0, 17, 0>;

        extern template struct change_recovery_account_operation<0, 16, 0>;
        extern template struct change_recovery_account_operation<0, 17, 0>;

        extern template struct decline_voting_rights_operation<0, 16, 0>;
        extern template struct decline_voting_rights_operation<0, 17, 0>;

        extern template struct delegate_vesting_shares_operation<0, 16, 0>;
        extern template struct delegate_vesting_shares_operation<0, 17, 0>;
    }
}