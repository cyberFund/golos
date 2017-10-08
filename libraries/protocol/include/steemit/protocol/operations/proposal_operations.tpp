namespace steemit {
    namespace protocol {
        extern template struct proposal_create_operation<0, 16, 0>;
        extern template struct proposal_create_operation<0, 17, 0>;

        extern template struct proposal_update_operation<0, 16, 0>;
        extern template struct proposal_update_operation<0, 17, 0>;

        extern template struct proposal_delete_operation<0, 16, 0>;
        extern template struct proposal_delete_operation<0, 17, 0>;
    }
}