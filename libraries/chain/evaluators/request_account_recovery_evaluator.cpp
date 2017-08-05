#include <steemit/chain/evaluators/request_account_recovery_evaluator.hpp>
void steemit::chain::request_account_recovery_evaluator::do_apply(const protocol::request_account_recovery_operation &o) {

    const auto &account_to_recover = this->_db.get_account(o.account_to_recover);

    if (account_to_recover.recovery_account.length()) {   // Make sure recovery matches expected recovery account
        FC_ASSERT(account_to_recover.recovery_account ==
                  o.recovery_account,
                  "Cannot recover an account that does not have you as there recovery partner.");
    } else {                                                  // Empty string recovery account defaults to top witness
        FC_ASSERT(
                this->_db.get_index<witness_index>().indices().get<by_vote_name>().begin()->owner ==
                o.recovery_account, "Top witness must recover an account with no recovery partner.");
    }

    const auto &recovery_request_idx = this->_db.get_index<account_recovery_request_index>().indices().get<by_account>();
    auto request = recovery_request_idx.find(o.account_to_recover);

    if (request == recovery_request_idx.end()) // New Request
    {
        FC_ASSERT(!o.new_owner_authority.is_impossible(), "Cannot recover using an impossible authority.");
        FC_ASSERT(o.new_owner_authority.weight_threshold, "Cannot recover using an open authority.");

        // Check accounts in the new authority exist
        if ((this->_db.has_hardfork(STEEMIT_HARDFORK_0_15__465) ||
             this->_db.is_producing())) {
            for (auto &a : o.new_owner_authority.account_auths) {
                this->_db.get_account(a.first);
            }
        }

        this->_db.create<account_recovery_request_object>([&](account_recovery_request_object &req) {
            req.account_to_recover = o.account_to_recover;
            req.new_owner_authority = o.new_owner_authority;
            req.expires = this->_db.head_block_time() +
                          STEEMIT_ACCOUNT_RECOVERY_REQUEST_EXPIRATION_PERIOD;
        });
    } else if (o.new_owner_authority.weight_threshold ==
               0) // Cancel Request if authority is open
    {
        this->_db.remove(*request);
    } else // Change Request
    {
        FC_ASSERT(!o.new_owner_authority.is_impossible(), "Cannot recover using an impossible authority.");

        // Check accounts in the new authority exist
        if ((this->_db.has_hardfork(STEEMIT_HARDFORK_0_15__465) ||
             this->_db.is_producing())) {
            for (auto &a : o.new_owner_authority.account_auths) {
                this->_db.get_account(a.first);
            }
        }

        this->_db.modify(*request, [&](account_recovery_request_object &req) {
            req.new_owner_authority = o.new_owner_authority;
            req.expires = this->_db.head_block_time() +
                          STEEMIT_ACCOUNT_RECOVERY_REQUEST_EXPIRATION_PERIOD;
        });
    }
}
