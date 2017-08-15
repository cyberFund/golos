#include <steemit/chain/evaluators/transfer_to_vesting_evaluator.hpp>

void steemit::chain::transfer_to_vesting_evaluator::do_apply(const transfer_to_vesting_operation &o) {

    const auto &from_account = this->_db.get_account(o.from);
    const auto &to_account = o.to.size() ? this->_db.get_account(o.to) : from_account;
    FC_ASSERT(this->_db.get_balance(from_account, STEEM_SYMBOL) >= o.amount,
              "Account does not have sufficient GOLOS for transfer.");
    this->_db.adjust_balance(from_account, -o.amount);
    this->_db.create_vesting(to_account, o.amount);
}
