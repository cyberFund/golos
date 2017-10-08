namespace steemit {
    namespace protocol {
        extern template struct comment_operation<0, 16, 0>;
        extern template struct comment_operation<0, 17, 0>;

        extern template struct comment_options_operation<0, 16, 0>;
        extern template struct comment_options_operation<0, 17, 0>;

        extern template struct comment_payout_extension_operation<0, 16, 0>;
        extern template struct comment_payout_extension_operation<0, 17, 0>;

        extern template struct delete_comment_operation<0, 16, 0>;
        extern template struct delete_comment_operation<0, 17, 0>;
    }
}