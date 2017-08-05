#include <steemit/chain/evaluators/feed_publish_evaluator.hpp>
void steemit::chain::feed_publish_evaluator::do_apply(const protocol::feed_publish_operation &o) {

    const auto &witness = this->_db.get_witness(o.publisher);
    this->_db.modify(witness, [&](witness_object &w) {
        w.sbd_exchange_rate = o.exchange_rate;
        w.last_sbd_exchange_update = this->_db.head_block_time();
    });
}
