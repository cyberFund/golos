#include <steemit/chain/evaluators/challenge_authority_evaluator.hpp>

void steemit::chain::challenge_authority_evaluator::do_apply(const protocol::challenge_authority_operation &o) {

    if (this->_db.has_hardfork(STEEMIT_HARDFORK_0_14__307)) {
        FC_ASSERT(false, "Challenge authority operation is currently disabled.");
    }
    const auto &challenged = this->_db.get_account(o.challenged);
    const auto &challenger = this->_db.get_account(o.challenger);

    if (o.require_owner) {
        FC_ASSERT(challenged.reset_account == o.challenger,
                  "Owner authority can only be challenged by its reset account.");
        FC_ASSERT(challenger.balance >= STEEMIT_OWNER_CHALLENGE_FEE);
        FC_ASSERT(!challenged.owner_challenged);
        FC_ASSERT(this->_db.head_block_time() - challenged.last_owner_proved > STEEMIT_OWNER_CHALLENGE_COOLDOWN);

        this->_db.adjust_balance(challenger, -STEEMIT_OWNER_CHALLENGE_FEE);
        this->_db.create_vesting(this->_db.get_account(o.challenged), STEEMIT_OWNER_CHALLENGE_FEE);

        this->_db.modify(challenged, [&](account_object &a) {
            a.owner_challenged = true;
        });
    } else {
        FC_ASSERT(challenger.balance >= STEEMIT_ACTIVE_CHALLENGE_FEE,
                  "Account does not have sufficient funds to pay challenge fee.");
        FC_ASSERT(!(challenged.owner_challenged || challenged.active_challenged), "Account is already challenged.");
        FC_ASSERT(this->_db.head_block_time() - challenged.last_active_proved > STEEMIT_ACTIVE_CHALLENGE_COOLDOWN,
                  "Account cannot be challenged because it was recently challenged.");

        this->_db.adjust_balance(challenger, -STEEMIT_ACTIVE_CHALLENGE_FEE);
        this->_db.create_vesting(this->_db.get_account(o.challenged), STEEMIT_ACTIVE_CHALLENGE_FEE);

        this->_db.modify(challenged, [&](account_object &a) {
            a.active_challenged = true;
        });
    }
}
