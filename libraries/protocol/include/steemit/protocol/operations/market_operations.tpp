namespace steemit {
namespace protocol {
extern template struct convert_operation<0, 16, 0>;
extern template struct convert_operation<0, 17, 0>;

extern template struct limit_order_create_operation<0, 16, 0>;
extern template struct limit_order_create_operation<0, 17, 0>;

extern template struct limit_order_create2_operation<0, 16, 0>;
extern template struct limit_order_create2_operation<0, 17, 0>;

extern template struct limit_order_cancel_operation<0, 16, 0>;
extern template struct limit_order_cancel_operation<0, 17, 0>;

extern template struct call_order_update_operation<0, 16, 0>;
extern template struct call_order_update_operation<0, 17, 0>;

extern template struct bid_collateral_operation<0, 16, 0>;
extern template struct bid_collateral_operation<0, 17, 0>;

}
}