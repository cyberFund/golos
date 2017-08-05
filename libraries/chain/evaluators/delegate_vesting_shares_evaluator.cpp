#include <steemit/chain/evaluators/delegate_vesting_shares_evaluator.hpp>
void steemit::chain::delegate_vesting_shares_evaluator::do_apply(const protocol::delegate_vesting_shares_operation &op) {

    FC_ASSERT(this->_db.has_hardfork(STEEMIT_HARDFORK_0_17__101),
              "delegate_vesting_shares_operation is not enabled until HF 17"); //TODO: Delete after hardfork

    const auto &delegator = this->_db.get_account(op.delegator);
    const auto &delegatee = this->_db.get_account(op.delegatee);
    auto delegation = this->_db.find<vesting_delegation_object, by_delegation>(
            boost::make_tuple(op.delegator, op.delegatee));

    auto available_shares = delegator.vesting_shares -
                            delegator.delegated_vesting_shares -
                            asset(delegator.to_withdraw -
                                  delegator.withdrawn, VESTS_SYMBOL);

    const auto &wso = this->_db.get_witness_schedule_object();
    const auto &gpo = this->_db.get_dynamic_global_properties();
    auto min_delegation =
            asset(wso.median_props.account_creation_fee.amount *
                  10, STEEM_SYMBOL) * gpo.get_vesting_share_price();
    auto min_update = wso.median_props.account_creation_fee *
                      gpo.get_vesting_share_price();

    // If delegation doesn't exist, create it
    if (delegation == nullptr) {
        FC_ASSERT(available_shares >=
                  op.vesting_shares, "Account does not have enough vesting shares to delegate.");
        FC_ASSERT(op.vesting_shares >=
                  min_delegation, "Account must delegate a minimum of ${v}", ("v", min_delegation));

        this->_db.create<vesting_delegation_object>([&](vesting_delegation_object &obj) {
            obj.delegator = op.delegator;
            obj.delegatee = op.delegatee;
            obj.vesting_shares = op.vesting_shares;
            obj.min_delegation_time = this->_db.head_block_time();
        });

        this->_db.modify(delegator, [&](account_object &a) {
            a.delegated_vesting_shares += op.vesting_shares;
        });

        this->_db.modify(delegatee, [&](account_object &a) {
            a.received_vesting_shares += op.vesting_shares;
        });
    } else if (op.vesting_shares >= delegation->vesting_shares) {
        auto delta = op.vesting_shares - delegation->vesting_shares;

        FC_ASSERT(delta >=
                  min_update, "Steem Power increase is not enough of a different. min_update: ${min}",
                  ("min", min_update));
        FC_ASSERT(available_shares >= op.vesting_shares -
                                      delegation->vesting_shares,
                  "Account does not have enough vesting shares to delegate.");

        this->_db.modify(delegator, [&](account_object &a) {
            a.delegated_vesting_shares += delta;
        });

        this->_db.modify(delegatee, [&](account_object &a) {
            a.received_vesting_shares += delta;
        });

        this->_db.modify(*delegation, [&](vesting_delegation_object &obj) {
            obj.vesting_shares = op.vesting_shares;
        });
    } else {
        auto delta = delegation->vesting_shares - op.vesting_shares;

        FC_ASSERT(delta >= min_update,
                  "Steem Power increase is not enough of a different. min_update: ${min}",
                  ("min", min_update));
        FC_ASSERT(op.vesting_shares >= min_delegation ||
                  op.vesting_shares.amount ==
                  0, "Delegation must be removed or leave minimum delegation amount of ${v}",
                  ("v", min_delegation));

        this->_db.create<vesting_delegation_expiration_object>(
                [&](vesting_delegation_expiration_object &obj) {
                    obj.delegator = op.delegator;
                    obj.vesting_shares = delta;
                    obj.expiration = std::max(this->_db.head_block_time() +
                                              STEEMIT_CASHOUT_WINDOW_SECONDS,
                                              delegation->min_delegation_time);

                });

        this->_db.modify(delegatee, [&](account_object &a) {
            a.received_vesting_shares -= delta;
        });

        if (op.vesting_shares.amount > 0) {
            this->_db.modify(*delegation, [&](vesting_delegation_object &obj) {
                obj.vesting_shares = op.vesting_shares;
            });
        } else {
            this->_db.remove(*delegation);
        }
    }
}
