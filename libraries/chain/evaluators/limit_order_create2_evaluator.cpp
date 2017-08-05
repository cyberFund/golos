#include <steemit/chain/evaluators/limit_order_create2_evaluator.hpp>
void steemit::chain::limit_order_create2_evaluator::do_apply(const limit_order_create2_operation &o) {

    FC_ASSERT(o.expiration >
              this->_db.head_block_time(), "Limit order has to expire after head block time.");

    const auto &owner = this->_db.get_account(o.owner);

    FC_ASSERT(this->_db.get_balance(owner, o.amount_to_sell.symbol) >=
              o.amount_to_sell, "Account does not have sufficient funds for limit order.");

    this->_db.adjust_balance(owner, -o.amount_to_sell);

    const auto &order = this->_db.create<limit_order_object>([&](limit_order_object &obj) {
        obj.created = this->_db.head_block_time();
        obj.seller = o.owner;
        obj.orderid = o.orderid;
        obj.for_sale = o.amount_to_sell.amount;
        obj.sell_price = o.exchange_rate;
        obj.expiration = o.expiration;
    });

    bool filled = this->_db.apply_order(order);

    if (o.fill_or_kill) {
        FC_ASSERT(filled, "Cancelling order because it was not filled.");
    }
}
