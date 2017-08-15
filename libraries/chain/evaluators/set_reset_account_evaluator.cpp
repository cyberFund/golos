#include <steemit/chain/evaluators/set_reset_account_evaluator.hpp>

void steemit::chain::set_reset_account_evaluator::do_apply(const protocol::set_reset_account_operation &op) {

    FC_ASSERT(false, "Set Reset Account Operation is currently disabled.");

    const auto &acnt = this->_db.get_account(op.account);
    this->_db.get_account(op.reset_account);

    FC_ASSERT(acnt.reset_account == op.current_reset_account,
              "Current reset account does not match reset account on account.");
    FC_ASSERT(acnt.reset_account != op.reset_account, "Reset account must change");

    this->_db.modify(acnt, [&](account_object &a) {
        a.reset_account = op.reset_account;
    });
}
