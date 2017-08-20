#include <steemit/chain/evaluators/withdraw_vesting_evaluator.hpp>

void steemit::chain::withdraw_vesting_evaluator::do_apply(const protocol::withdraw_vesting_operation &o) {


    const auto &account = this->_db.get_account(o.account);

    FC_ASSERT(account.vesting_shares >= asset(0, VESTS_SYMBOL),
              "Account does not have sufficient Golos Power for withdraw.");
    FC_ASSERT(account.vesting_shares - account.delegated_vesting_shares >= o.vesting_shares,
              "Account does not have sufficient Steem Power for withdraw.");

    if (!account.mined && this->_db.has_hardfork(STEEMIT_HARDFORK_0_1)) {
        const auto &props = this->_db.get_dynamic_global_properties();
        const witness_schedule_object &wso = this->_db.get_witness_schedule_object();

        asset min_vests = wso.median_props.account_creation_fee * props.get_vesting_share_price();
        min_vests.amount.value *= 10;

        FC_ASSERT(account.vesting_shares > min_vests ||
                  (this->_db.has_hardfork(STEEMIT_HARDFORK_0_16__562) && o.vesting_shares.amount == 0),
                  "Account registered by another account requires 10x account creation fee worth of Golos Power before it can be powered down.");
    }

    if (o.vesting_shares.amount == 0) {
        if (this->_db.is_producing() || this->_db.has_hardfork(STEEMIT_HARDFORK_0_5__57)) {
            FC_ASSERT(account.vesting_withdraw_rate.amount != 0,
                      "This operation would not change the vesting withdraw rate.");
        }

        this->_db.modify(account, [&](account_object &a) {
            a.vesting_withdraw_rate = asset(0, VESTS_SYMBOL);
            a.next_vesting_withdrawal = time_point_sec::maximum();
            a.to_withdraw = 0;
            a.withdrawn = 0;
        });
    } else {
        int vesting_withdraw_intervals = 0;

        if (this->_db.has_hardfork(STEEMIT_HARDFORK_0_17__103)) {
            vesting_withdraw_intervals = STEEMIT_VESTING_WITHDRAW_INTERVALS;
        } else if (this->_db.has_hardfork(STEEMIT_HARDFORK_0_16__551)) {
            vesting_withdraw_intervals = STEEMIT_VESTING_WITHDRAW_INTERVALS_PRE_HF17;
        } else {
            vesting_withdraw_intervals = STEEMIT_VESTING_WITHDRAW_INTERVALS_PRE_HF16;
        }

        this->_db.modify(account, [&](account_object &a) {
            auto new_vesting_withdraw_rate = asset(o.vesting_shares.amount / vesting_withdraw_intervals, VESTS_SYMBOL);

            if (new_vesting_withdraw_rate.amount == 0) {
                new_vesting_withdraw_rate.amount = 1;
            }

            if (this->_db.is_producing() || this->_db.has_hardfork(STEEMIT_HARDFORK_0_5__57)) {
                FC_ASSERT(account.vesting_withdraw_rate != new_vesting_withdraw_rate,
                          "This operation would not change the vesting withdraw rate.");
            }

            a.vesting_withdraw_rate = new_vesting_withdraw_rate;
            a.next_vesting_withdrawal =
                    this->_db.head_block_time() + fc::seconds(STEEMIT_VESTING_WITHDRAW_INTERVAL_SECONDS);
            a.to_withdraw = o.vesting_shares.amount;
            a.withdrawn = 0;
        });
    }
}
