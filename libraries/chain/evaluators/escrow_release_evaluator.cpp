#include <steemit/chain/evaluators/escrow_release_evaluator.hpp>

void steemit::chain::escrow_release_evaluator::do_apply(const protocol::escrow_release_operation &o) {
    try {

        this->_db.get_account(o.from); // Verify from account exists
        const auto &receiver_account = this->_db.get_account(o.receiver);

        const auto &e = this->_db.get_escrow(o.from, o.escrow_id);
        FC_ASSERT(e.steem_balance >= o.steem_amount,
                  "Release amount exceeds escrow balance. Amount: ${a}, Balance: ${b}",
                  ("a", o.steem_amount)("b", e.steem_balance));
        FC_ASSERT(e.sbd_balance >= o.sbd_amount, "Release amount exceeds escrow balance. Amount: ${a}, Balance: ${b}",
                  ("a", o.sbd_amount)("b", e.sbd_balance));
        FC_ASSERT(e.to == o.to, "Operation 'to' (${o}) does not match escrow 'to' (${e}).", ("o", o.to)("e", e.to));
        FC_ASSERT(e.agent == o.agent, "Operation 'agent' (${a}) does not match escrow 'agent' (${e}).",
                  ("o", o.agent)("e", e.agent));
        FC_ASSERT(o.receiver == e.from || o.receiver == e.to, "Funds must be released to 'from' (${f}) or 'to' (${t})",
                  ("f", e.from)("t", e.to));
        FC_ASSERT(e.to_approved && e.agent_approved, "Funds cannot be released prior to escrow approval.");

        // If there is a dispute regardless of expiration, the agent can release funds to either party
        if (e.disputed) {
            FC_ASSERT(o.who == e.agent, "Only 'agent' (${a}) can release funds in a disputed escrow.", ("a", e.agent));
        } else {
            FC_ASSERT(o.who == e.from || o.who == e.to,
                      "Only 'from' (${f}) and 'to' (${t}) can release funds from a non-disputed escrow",
                      ("f", e.from)("t", e.to));

            if (e.escrow_expiration > this->_db.head_block_time()) {
                // If there is no dispute and escrow has not expired, either party can release funds to the other.
                if (o.who == e.from) {
                    FC_ASSERT(o.receiver == e.to, "Only 'from' (${f}) can release funds to 'to' (${t}).",
                              ("f", e.from)("t", e.to));
                } else if (o.who == e.to) {
                    FC_ASSERT(o.receiver == e.from, "Only 'to' (${t}) can release funds to 'from' (${t}).",
                              ("f", e.from)("t", e.to));
                }
            }
        }
        // If escrow expires and there is no dispute, either party can release funds to either party.

        this->_db.adjust_balance(receiver_account, o.steem_amount);
        this->_db.adjust_balance(receiver_account, o.sbd_amount);

        this->_db.modify(e, [&](escrow_object &esc) {
            esc.steem_balance -= o.steem_amount;
            esc.sbd_balance -= o.sbd_amount;
        });

        if (e.steem_balance.amount == 0 && e.sbd_balance.amount == 0) {
            this->_db.remove(e);
        }
    } FC_CAPTURE_AND_RETHROW((o))
}
