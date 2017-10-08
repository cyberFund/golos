namespace steemit {
    namespace protocol {
        extern template struct asset_options<0, 16, 0>;
        extern template struct asset_options<0, 17, 0>;

        extern template struct asset_create_operation<0, 16, 0>;
        extern template struct asset_create_operation<0, 17, 0>;

        extern template struct asset_global_settle_operation<0, 16, 0>;
        extern template struct asset_global_settle_operation<0, 17, 0>;

        extern template struct asset_settle_operation<0, 16, 0>;
        extern template struct asset_settle_operation<0, 17, 0>;

        extern template struct asset_force_settle_operation<0, 16, 0>;
        extern template struct asset_force_settle_operation<0, 17, 0>;

        extern template struct asset_fund_fee_pool_operation<0, 16, 0>;
        extern template struct asset_fund_fee_pool_operation<0, 17, 0>;

        extern template struct asset_update_operation<0, 16, 0>;
        extern template struct asset_update_operation<0, 17, 0>;

        extern template struct asset_update_bitasset_operation<0, 16, 0>;
        extern template struct asset_update_bitasset_operation<0, 17, 0>;

        extern template struct asset_update_feed_producers_operation<0, 16, 0>;
        extern template struct asset_update_feed_producers_operation<0, 17, 0>;

        extern template struct asset_publish_feed_operation<0, 16, 0>;
        extern template struct asset_publish_feed_operation<0, 17, 0>;

        extern template struct asset_issue_operation<0, 16, 0>;
        extern template struct asset_issue_operation<0, 17, 0>;

        extern template struct asset_reserve_operation<0, 16, 0>;
        extern template struct asset_reserve_operation<0, 17, 0>;

        extern template struct asset_claim_fees_operation<0, 16, 0>;
        extern template struct asset_claim_fees_operation<0, 17, 0>;
    }
}