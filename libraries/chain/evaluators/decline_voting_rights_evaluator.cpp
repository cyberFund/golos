#include <steemit/chain/evaluators/decline_voting_rights_evaluator.hpp>
void steemit::chain::decline_voting_rights_evaluator::do_apply(const protocol::decline_voting_rights_operation &o) {

    FC_ASSERT(this->_db.has_hardfork(STEEMIT_HARDFORK_0_14__324));

    const auto &account = this->_db.get_account(o.account);
    const auto &request_idx = this->_db.get_index<decline_voting_rights_request_index>().indices().get<by_account>();
    auto itr = request_idx.find(account.id);

    if (o.decline) {
        FC_ASSERT(itr ==
                  request_idx.end(), "Cannot create new request because one already exists.");

        this->_db.create<decline_voting_rights_request_object>(
                [&](decline_voting_rights_request_object &req) {
                    req.account = account.id;
                    req.effective_date = this->_db.head_block_time() +
                                         STEEMIT_OWNER_AUTH_RECOVERY_PERIOD;
                });
    } else {
        FC_ASSERT(itr !=
                  request_idx.end(), "Cannot cancel the request because it does not exist.");
        this->_db.remove(*itr);
    }
}
