
#include <steemit/chain/evaluators/account_create_evaluator.hpp>

void steemit::chain::account_create_evaluator::do_apply(const account_create_operation &o) {

    const auto &creator = this->_db.get_account(o.creator);

    const auto &props = this->_db.get_dynamic_global_properties();

    FC_ASSERT(creator.balance >=
              o.fee, "Insufficient balance to create account.", ("creator.balance", creator.balance)("required", o.fee));

    if (this->_db.has_hardfork(STEEMIT_HARDFORK_0_17__101)) {
        const witness_schedule_object &wso = this->_db.get_witness_schedule_object();
        FC_ASSERT(o.fee >= wso.median_props.account_creation_fee *
                           asset(STEEMIT_CREATE_ACCOUNT_WITH_STEEM_MODIFIER, STEEM_SYMBOL), "Insufficient Fee: ${f} required, ${p} provided.",
                  ("f", wso.median_props.account_creation_fee *
                        asset(STEEMIT_CREATE_ACCOUNT_WITH_STEEM_MODIFIER, STEEM_SYMBOL))
                          ("p", o.fee));
    } else if (this->_db.has_hardfork(STEEMIT_HARDFORK_0_1)) {
        const witness_schedule_object &wso = this->_db.get_witness_schedule_object();
        FC_ASSERT(o.fee >=
                  wso.median_props.account_creation_fee, "Insufficient Fee: ${f} required, ${p} provided.",
                  ("f", wso.median_props.account_creation_fee)
                          ("p", o.fee));
    }

    if (this->_db.is_producing() ||
        this->_db.has_hardfork(STEEMIT_HARDFORK_0_15__465)) {
        for (auto &a : o.owner.account_auths) {
            this->_db.get_account(a.first);
        }

        for (auto &a : o.active.account_auths) {
            this->_db.get_account(a.first);
        }

        for (auto &a : o.posting.account_auths) {
            this->_db.get_account(a.first);
        }
    }

    this->_db.modify(creator, [&](account_object &c) {
        c.balance -= o.fee;
    });

    const auto &new_account = this->_db. template create<account_object>([&](account_object &acc) {
        acc.name = o.new_account_name;
        acc.memo_key = o.memo_key;
        acc.created = props.time;
        acc.last_vote_time = props.time;
        acc.mined = false;

        if (!this->_db.has_hardfork(STEEMIT_HARDFORK_0_11__169)) {
            acc.recovery_account = STEEMIT_INIT_MINER_NAME;
        } else {
            acc.recovery_account = o.creator;
        }


#ifndef IS_LOW_MEM
        from_string(acc.json_metadata, o.json_metadata);
#endif
    });

    this->_db. template create<account_authority_object>([&](account_authority_object &auth) {
        auth.account = o.new_account_name;
        auth.owner = o.owner;
        auth.active = o.active;
        auth.posting = o.posting;
        auth.last_owner_update = fc::time_point_sec::min();
    });

    if (o.fee.amount > 0) {
        this->_db.create_vesting(new_account, o.fee);
    }
}
