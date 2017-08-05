#include <steemit/chain/evaluators/limit_order_cancel_evaluator.hpp>
void steemit::chain::limit_order_cancel_evaluator::do_apply(const limit_order_cancel_operation &o) {

    this->_db.cancel_order(this->_db.get_limit_order(o.owner, o.orderid));
}
