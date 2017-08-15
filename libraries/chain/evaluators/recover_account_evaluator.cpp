#include <steemit/chain/evaluators/recover_account_evaluator.hpp>

void steemit::chain::recover_account_evaluator::do_apply(const protocol::recover_account_operation &o) {

    const auto &account = this->_db.get_account(o.account_to_recover);

    if (this->_db.has_hardfork(STEEMIT_HARDFORK_0_12)) {
        FC_ASSERT(this->_db.head_block_time() - account.last_account_recovery > STEEMIT_OWNER_UPDATE_LIMIT,
                  "Owner authority can only be updated once an hour.");
    }

    const auto &recovery_request_idx = this->_db.get_index<account_recovery_request_index>().indices().get<
            by_account>();
    auto request = recovery_request_idx.find(o.account_to_recover);

    FC_ASSERT(request != recovery_request_idx.end(), "There are no active recovery requests for this account.");
    FC_ASSERT(request->new_owner_authority == o.new_owner_authority,
              "New owner authority does not match recovery request.");

    const auto &recent_auth_idx = this->_db.get_index<owner_authority_history_index>().indices().get<by_account>();
    auto hist = recent_auth_idx.lower_bound(o.account_to_recover);
    bool found = false;

    while (hist != recent_auth_idx.end() && hist->account == o.account_to_recover && !found) {
        found = hist->previous_owner_authority == o.recent_owner_authority;
        if (found) {
            break;
        }
        ++hist;
    }

    FC_ASSERT(found, "Recent authority not found in authority history.");

    this->_db.remove(*request); // Remove first, update_owner_authority may invalidate iterator
    this->_db.update_owner_authority(account, o.new_owner_authority);
    this->_db.modify(account, [&](account_object &a) {
        a.last_account_recovery = this->_db.head_block_time();
    });
}
