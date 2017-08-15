#include <steemit/chain/evaluators/transfer_to_savings_evaluator.hpp>

void steemit::chain::transfer_to_savings_evaluator::do_apply(const transfer_to_savings_operation &op) {

    const auto &from = this->_db.get_account(op.from);
    const auto &to = this->_db.get_account(op.to);
    FC_ASSERT(this->_db.get_balance(from, op.amount.symbol) >= op.amount,
              "Account does not have sufficient funds to transfer to savings.");

    this->_db.adjust_balance(from, -op.amount);
    this->_db.adjust_savings_balance(to, op.amount);
}
