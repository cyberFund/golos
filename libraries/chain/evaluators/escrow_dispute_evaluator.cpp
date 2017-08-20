#include <steemit/chain/evaluators/escrow_dispute_evaluator.hpp>

void steemit::chain::escrow_dispute_evaluator::do_apply(const protocol::escrow_dispute_operation &o) {
    try {

        this->_db.get_account(o.from); // Verify from account exists

        const auto &e = this->_db.get_escrow(o.from, o.escrow_id);
        FC_ASSERT(this->_db.head_block_time() < e.escrow_expiration,
                  "Disputing the escrow must happen before expiration.");
        FC_ASSERT(e.to_approved && e.agent_approved,
                  "The escrow must be approved by all parties before a dispute can be raised.");
        FC_ASSERT(!e.disputed, "The escrow is already under dispute.");
        FC_ASSERT(e.to == o.to, "Operation 'to' (${o}) does not match escrow 'to' (${e}).", ("o", o.to)("e", e.to));
        FC_ASSERT(e.agent == o.agent, "Operation 'agent' (${a}) does not match escrow 'agent' (${e}).",
                  ("o", o.agent)("e", e.agent));

        this->_db.modify(e, [&](escrow_object &esc) {
            esc.disputed = true;
        });
    } FC_CAPTURE_AND_RETHROW((o))
}
