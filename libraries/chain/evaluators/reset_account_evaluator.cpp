#include <steemit/chain/evaluators/reset_account_evaluator.hpp>

void steemit::chain::reset_account_evaluator::do_apply(const protocol::reset_account_operation &op) {

    FC_ASSERT(false, "Reset Account Operation is currently disabled.");

    const auto &acnt = this->_db.get_account(op.account_to_reset);
    auto band = this->_db.find<account_bandwidth_object, by_account_bandwidth_type>(
            boost::make_tuple(op.account_to_reset, bandwidth_type::old_forum));
    if (band != nullptr) {
        FC_ASSERT((this->_db.head_block_time() - band->last_bandwidth_update) > fc::days(60),
                  "Account must be inactive for 60 days to be eligible for reset");
    }
    FC_ASSERT(acnt.reset_account == op.reset_account, "Reset account does not match reset account on account.");

    this->_db.update_owner_authority(acnt, op.new_owner_authority);
}
