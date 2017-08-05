#include <steemit/chain/evaluators/account_update_evaluator.hpp>

void steemit::chain::account_update_evaluator::do_apply(const account_update_operation &o) {

    if (this->_db.has_hardfork(STEEMIT_HARDFORK_0_1)) {
        FC_ASSERT(o.account !=
                  STEEMIT_TEMP_ACCOUNT, "Cannot update temp account.");
    }

    if ((this->_db.has_hardfork(STEEMIT_HARDFORK_0_15__465) ||
         this->_db.is_producing()) && o.posting) { // TODO: Add HF 15
        o.posting->validate();
    }

    const auto &account = this->_db.get_account(o.account);
    const auto &account_auth = this->_db.template get<account_authority_object, by_account>(o.account);

    if (o.owner) {
#ifndef STEEMIT_BUILD_TESTNET
        if (this->_db.has_hardfork(STEEMIT_HARDFORK_0_11)) {
            FC_ASSERT(this->_db.head_block_time() -
                      account_auth.last_owner_update >
                      STEEMIT_OWNER_UPDATE_LIMIT, "Owner authority can only be updated once an hour.");
        }

#endif

        if ((this->_db.has_hardfork(STEEMIT_HARDFORK_0_15__465) ||
             this->_db.is_producing())) // TODO: Add HF 15
        {
            for (auto a: o.owner->account_auths) {
                this->_db.get_account(a.first);
            }
        }


        this->_db.update_owner_authority(account, *o.owner);
    }

    if (o.active &&
        (this->_db.has_hardfork(STEEMIT_HARDFORK_0_15__465) ||
         this->_db.is_producing())) // TODO: Add HF 15
    {
        for (auto a: o.active->account_auths) {
            this->_db.get_account(a.first);
        }
    }

    if (o.posting &&
        (this->_db.has_hardfork(STEEMIT_HARDFORK_0_15__465) ||
         this->_db.is_producing())) // TODO: Add HF 15
    {
        for (auto a: o.posting->account_auths) {
            this->_db.get_account(a.first);
        }
    }

    this->_db.modify(account, [&](account_object &acc) {
        if (o.memo_key != public_key_type()) {
            acc.memo_key = o.memo_key;
        }

        if ((o.active || o.owner) && acc.active_challenged) {
            acc.active_challenged = false;
            acc.last_active_proved = this->_db.head_block_time();
        }

        acc.last_account_update = this->_db.head_block_time();

#ifndef IS_LOW_MEM
        if (o.json_metadata.size() > 0) {
            from_string(acc.json_metadata, o.json_metadata);
        }
#endif
    });

    if (o.active || o.posting) {
        this->_db.modify(account_auth, [&](account_authority_object &auth) {
            if (o.active) {
                auth.active = *o.active;
            }
            if (o.posting) {
                auth.posting = *o.posting;
            }
        });
    }

}
