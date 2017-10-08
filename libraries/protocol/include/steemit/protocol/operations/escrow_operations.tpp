namespace steemit {
    namespace protocol {
        extern template escrow_transfer_operation<0, 16, 0>;
        extern template escrow_transfer_operation<0, 17, 0>;

        extern template escrow_approve_operation<0, 16, 0>;
        extern template escrow_approve_operation<0, 17, 0>;

        extern template escrow_dispute_operation<0, 16, 0>;
        extern template escrow_dispute_operation<0, 17, 0>;

        extern template escrow_release_operation<0, 16, 0>;
        extern template escrow_release_operation<0, 17, 0>;
    }
}