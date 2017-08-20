#include <steemit/chain/evaluators/convert_evaluator.hpp>

void steemit::chain::convert_evaluator::do_apply(const protocol::convert_operation &o) {

    const auto &owner = this->_db.get_account(o.owner);
    FC_ASSERT(this->_db.get_balance(owner, o.amount.symbol) >= o.amount,
              "Account does not have sufficient balance for conversion.");

    this->_db.adjust_balance(owner, -o.amount);

    const auto &fhistory = this->_db.get_feed_history();
    FC_ASSERT(!fhistory.current_median_history.is_null(), "Cannot convert SBD because there is no price feed.");

    auto steem_conversion_delay = STEEMIT_CONVERSION_DELAY_PRE_HF16;
    if (this->_db.has_hardfork(STEEMIT_HARDFORK_0_16__551)) {
        steem_conversion_delay = STEEMIT_CONVERSION_DELAY;
    }

    this->_db.create<convert_request_object>([&](convert_request_object &obj) {
        obj.owner = o.owner;
        obj.requestid = o.requestid;
        obj.amount = o.amount;
        obj.conversion_date = this->_db.head_block_time() + steem_conversion_delay;
    });

}
