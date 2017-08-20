#include <steemit/chain/evaluators/account_create_with_delegation_evaluator.hpp>

void steemit::chain::account_create_with_delegation_evaluator::do_apply(
        const account_create_with_delegation_operation &o) {

    FC_ASSERT(this->_db.has_hardfork(STEEMIT_HARDFORK_0_17__101),
              "Account creation with delegation is not enabled until hardfork 17");

    const auto &creator = this->_db.get_account(o.creator);
    const auto &props = this->_db.get_dynamic_global_properties();
    const witness_schedule_object &wso = this->_db.get_witness_schedule_object();

    FC_ASSERT(creator.balance >= o.fee, "Insufficient balance to create account.",
              ("creator.balance", creator.balance)("required", o.fee));

    FC_ASSERT(creator.vesting_shares - creator.delegated_vesting_shares -
              asset(creator.to_withdraw - creator.withdrawn, VESTS_SYMBOL) >= o.delegation,
              "Insufficient vesting shares to delegate to new account.",
              ("creator.vesting_shares", creator.vesting_shares)("creator.delegated_vesting_shares",
                                                                 creator.delegated_vesting_shares)("required",
                                                                                                   o.delegation));

    auto target_delegation =
            asset(wso.median_props.account_creation_fee.amount * STEEMIT_CREATE_ACCOUNT_WITH_STEEM_MODIFIER *
                  STEEMIT_CREATE_ACCOUNT_DELEGATION_RATIO, STEEM_SYMBOL) * props.get_vesting_share_price();

    auto current_delegation = asset(o.fee.amount * STEEMIT_CREATE_ACCOUNT_DELEGATION_RATIO, STEEM_SYMBOL) *
                              props.get_vesting_share_price() + o.delegation;

    FC_ASSERT(current_delegation >= target_delegation, "Inssufficient Delegation ${f} required, ${p} provided.",
              ("f", target_delegation)("p", current_delegation)("account_creation_fee",
                                                                wso.median_props.account_creation_fee)("o.fee", o.fee)(
                      "o.delegation", o.delegation));

    FC_ASSERT(o.fee >= wso.median_props.account_creation_fee, "Insufficient Fee: ${f} required, ${p} provided.",
              ("f", wso.median_props.account_creation_fee)("p", o.fee));

    for (auto &a : o.owner.account_auths) {
        this->_db.get_account(a.first);
    }

    for (auto &a : o.active.account_auths) {
        this->_db.get_account(a.first);
    }

    for (auto &a : o.posting.account_auths) {
        this->_db.get_account(a.first);
    }

    this->_db.modify(creator, [&](account_object &c) {
        c.balance -= o.fee;
        c.delegated_vesting_shares += o.delegation;
    });

    const auto &new_account = this->_db.template create<account_object>([&](account_object &acc) {
        acc.name = o.new_account_name;
        acc.memo_key = o.memo_key;
        acc.created = props.time;
        acc.last_vote_time = props.time;
        acc.mined = false;

        acc.recovery_account = o.creator;

        acc.received_vesting_shares = o.delegation;

#ifndef IS_LOW_MEM
        from_string(acc.json_metadata, o.json_metadata);
#endif
    });

    this->_db.template create<account_authority_object>([&](account_authority_object &auth) {
        auth.account = o.new_account_name;
        auth.owner = o.owner;
        auth.active = o.active;
        auth.posting = o.posting;
        auth.last_owner_update = fc::time_point_sec::min();
    });

    this->_db.template create<vesting_delegation_object>([&](vesting_delegation_object &vdo) {
        vdo.delegator = o.creator;
        vdo.delegatee = o.new_account_name;
        vdo.vesting_shares = o.delegation;
        vdo.min_delegation_time = this->_db.head_block_time() + STEEMIT_CREATE_ACCOUNT_DELEGATION_TIME;
    });

    if (o.fee.amount > 0) {
        this->_db.create_vesting(new_account, o.fee);
    }
}
