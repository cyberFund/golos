#include <steemit/chain/evaluators/escrow_approve_evaluator.hpp>
void steemit::chain::escrow_approve_evaluator::do_apply(const protocol::escrow_approve_operation &o) {
    try {

        const auto &escrow = this->_db.get_escrow(o.from, o.escrow_id);

        FC_ASSERT(escrow.to ==
                  o.to, "Operation 'to' (${o}) does not match escrow 'to' (${e}).",
                  ("o", o.to)("e", escrow.to));
        FC_ASSERT(escrow.agent ==
                  o.agent, "Operation 'agent' (${a}) does not match escrow 'agent' (${e}).",
                  ("o", o.agent)("e", escrow.agent));
        FC_ASSERT(escrow.ratification_deadline >=
                  this->_db.head_block_time(),
                  "The escrow ratification deadline has passed. Escrow can no longer be ratified.");

        bool reject_escrow = !o.approve;

        if (o.who == o.to) {
            FC_ASSERT(!escrow.to_approved, "Account 'to' (${t}) has already approved the escrow.",
                      ("t", o.to));

            if (!reject_escrow) {
                this->_db.modify(escrow, [&](escrow_object &esc) {
                    esc.to_approved = true;
                });
            }
        }
        if (o.who == o.agent) {
            FC_ASSERT(!escrow.agent_approved, "Account 'agent' (${a}) has already approved the escrow.",
                      ("a", o.agent));

            if (!reject_escrow) {
                this->_db.modify(escrow, [&](escrow_object &esc) {
                    esc.agent_approved = true;
                });
            }
        }

        if (reject_escrow) {
            const auto &from_account = this->_db.get_account(o.from);
            this->_db.adjust_balance(from_account, escrow.steem_balance);
            this->_db.adjust_balance(from_account, escrow.sbd_balance);
            this->_db.adjust_balance(from_account, escrow.pending_fee);

            this->_db.remove(escrow);
        } else if (escrow.to_approved && escrow.agent_approved) {
            const auto &agent_account = this->_db.get_account(o.agent);
            this->_db.adjust_balance(agent_account, escrow.pending_fee);

            this->_db.modify(escrow, [&](escrow_object &esc) {
                esc.pending_fee.amount = 0;
            });
        }
    }
    FC_CAPTURE_AND_RETHROW((o))
}
