#include <steemit/chain/evaluators/transfer_evaluator.hpp>
void steemit::chain::transfer_evaluator::do_apply(const transfer_operation &o) {

    const auto &from_account = this->_db.get_account(o.from);
    const auto &to_account = this->_db.get_account(o.to);

    if (from_account.active_challenged) {
        this->_db.modify(from_account, [&](account_object &a) {
            a.active_challenged = false;
            a.last_active_proved = this->_db.head_block_time();
        });
    }

    FC_ASSERT(this->_db.get_balance(from_account, o.amount.symbol) >=
              o.amount, "Account does not have sufficient funds for transfer.");
    this->_db.adjust_balance(from_account, -o.amount);
    this->_db.adjust_balance(to_account, o.amount);
}
