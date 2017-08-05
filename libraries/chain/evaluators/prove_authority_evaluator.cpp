#include <steemit/chain/evaluators/prove_authority_evaluator.hpp>
void steemit::chain::prove_authority_evaluator::do_apply(const protocol::prove_authority_operation &o) {

    const auto &challenged = this->_db.get_account(o.challenged);
    FC_ASSERT(challenged.owner_challenged ||
              challenged.active_challenged, "Account is not challeneged. No need to prove authority.");

    this->_db.modify(challenged, [&](account_object &a) {
        a.active_challenged = false;
        a.last_active_proved = this->_db.head_block_time();
        if (o.require_owner) {
            a.owner_challenged = false;
            a.last_owner_proved = this->_db.head_block_time();
        }
    });
}
