#include <steemit/chain/evaluators/change_recovery_account_evaluator.hpp>

void steemit::chain::change_recovery_account_evaluator::do_apply(const protocol::change_recovery_account_operation &o) {

    this->_db.get_account(o.new_recovery_account); // Simply validate account exists
    const auto &account_to_recover = this->_db.get_account(o.account_to_recover);

    const auto &change_recovery_idx = this->_db.get_index<change_recovery_account_request_index>().indices().get<
            by_account>();
    auto request = change_recovery_idx.find(o.account_to_recover);

    if (request == change_recovery_idx.end()) // New request
    {
        this->_db.create<change_recovery_account_request_object>([&](change_recovery_account_request_object &req) {
            req.account_to_recover = o.account_to_recover;
            req.recovery_account = o.new_recovery_account;
            req.effective_on = this->_db.head_block_time() + STEEMIT_OWNER_AUTH_RECOVERY_PERIOD;
        });
    } else if (account_to_recover.recovery_account != o.new_recovery_account) // Change existing request
    {
        this->_db.modify(*request, [&](change_recovery_account_request_object &req) {
            req.recovery_account = o.new_recovery_account;
            req.effective_on = this->_db.head_block_time() + STEEMIT_OWNER_AUTH_RECOVERY_PERIOD;
        });
    } else // Request exists and changing back to current recovery account
    {
        this->_db.remove(*request);
    }
}
