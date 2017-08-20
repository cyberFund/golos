#include <steemit/chain/evaluators/witness_update_evaluator.hpp>

void steemit::chain::witness_update_evaluator::do_apply(const protocol::witness_update_operation &o) {

    this->_db.get_account(o.owner); // verify owner exists

    if (this->_db.has_hardfork(STEEMIT_HARDFORK_0_1)) {
        FC_ASSERT(o.url.size() <= STEEMIT_MAX_WITNESS_URL_LENGTH, "URL is too long");
    } else if (o.url.size() > STEEMIT_MAX_WITNESS_URL_LENGTH) {
        // after HF, above check can be moved to validate() if reindex doesn't show this warning
        wlog("URL is too long in block ${b}", ("b", this->_db.head_block_num() + 1));
    }

    if (this->_db.has_hardfork(STEEMIT_HARDFORK_0_14__410)) {
        FC_ASSERT(o.props.account_creation_fee.symbol == STEEM_SYMBOL);
    } else if (o.props.account_creation_fee.symbol != STEEM_SYMBOL) {
        // after HF, above check can be moved to validate() if reindex doesn't show this warning
        wlog("Wrong fee symbol in block ${b}", ("b", this->_db.head_block_num() + 1));
    }

    const auto &by_witness_name_idx = this->_db.get_index<witness_index>().indices().get<by_name>();
    auto wit_itr = by_witness_name_idx.find(o.owner);
    if (wit_itr != by_witness_name_idx.end()) {
        this->_db.modify(*wit_itr, [&](witness_object &w) {
            from_string(w.url, o.url);
            w.signing_key = o.block_signing_key;
            w.props = o.props;
        });
    } else {
        this->_db.create<witness_object>([&](witness_object &w) {
            w.owner = o.owner;
            from_string(w.url, o.url);
            w.signing_key = o.block_signing_key;
            w.created = this->_db.head_block_time();
            w.props = o.props;
        });
    }
}
