#include <steemit/chain/evaluators/transfer_from_savings_evaluator.hpp>

void steemit::chain::transfer_from_savings_evaluator::do_apply(const transfer_from_savings_operation &op) {

    const auto &from = this->_db.get_account(op.from);
    this->_db.get_account(op.to); // Verify to account exists

    FC_ASSERT(from.savings_withdraw_requests < STEEMIT_SAVINGS_WITHDRAW_REQUEST_LIMIT,
              "Account has reached limit for pending withdraw requests.");

    FC_ASSERT(this->_db.get_savings_balance(from, op.amount.symbol) >= op.amount);
    this->_db.adjust_savings_balance(from, -op.amount);
    this->_db.create<savings_withdraw_object>([&](savings_withdraw_object &s) {
        s.from = op.from;
        s.to = op.to;
        s.amount = op.amount;
#ifndef IS_LOW_MEM
        from_string(s.memo, op.memo);
#endif
        s.request_id = op.request_id;
        s.complete = this->_db.head_block_time() + STEEMIT_SAVINGS_WITHDRAW_TIME;
    });

    this->_db.modify(from, [&](account_object &a) {
        a.savings_withdraw_requests++;
    });
}
