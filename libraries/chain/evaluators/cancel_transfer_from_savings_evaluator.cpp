#include <steemit/chain/evaluators/cancel_transfer_from_savings_evaluator.hpp>
void steemit::chain::cancel_transfer_from_savings_evaluator::do_apply(const cancel_transfer_from_savings_operation &op) {

    const auto &swo = this->_db.get_savings_withdraw(op.from, op.request_id);
    this->_db.adjust_savings_balance(this->_db.get_account(swo.from), swo.amount);
    this->_db.remove(swo);

    const auto &from = this->_db.get_account(op.from);
    this->_db.modify(from, [&](account_object &a) {
        a.savings_withdraw_requests--;
    });
}
