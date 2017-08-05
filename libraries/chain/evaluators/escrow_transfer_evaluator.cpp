#include <steemit/chain/evaluators/escrow_transfer_evaluator.hpp>
void steemit::chain::escrow_transfer_evaluator::do_apply(const protocol::escrow_transfer_operation &o) {
    try {


        const auto &from_account = this->_db.get_account(o.from);
        this->_db.get_account(o.to);
        this->_db.get_account(o.agent);

        FC_ASSERT(o.ratification_deadline >
                  this->_db.head_block_time(),
                  "The escorw ratification deadline must be after head block time.");
        FC_ASSERT(o.escrow_expiration >
                  this->_db.head_block_time(), "The escrow expiration must be after head block time.");

        asset steem_spent = o.steem_amount;
        asset sbd_spent = o.sbd_amount;
        if (o.fee.symbol == STEEM_SYMBOL) {
            steem_spent += o.fee;
        } else {
            sbd_spent += o.fee;
        }

        FC_ASSERT(from_account.balance >=
                  steem_spent, "Account cannot cover STEEM costs of escrow. Required: ${r} Available: ${a}",
                  ("r", steem_spent)("a", from_account.balance));
        FC_ASSERT(from_account.sbd_balance >=
                  sbd_spent, "Account cannot cover SBD costs of escrow. Required: ${r} Available: ${a}",
                  ("r", sbd_spent)("a", from_account.sbd_balance));

        this->_db.adjust_balance(from_account, -steem_spent);
        this->_db.adjust_balance(from_account, -sbd_spent);

        this->_db.create<escrow_object>([&](escrow_object &esc) {
            esc.escrow_id = o.escrow_id;
            esc.from = o.from;
            esc.to = o.to;
            esc.agent = o.agent;
            esc.ratification_deadline = o.ratification_deadline;
            esc.escrow_expiration = o.escrow_expiration;
            esc.sbd_balance = o.sbd_amount;
            esc.steem_balance = o.steem_amount;
            esc.pending_fee = o.fee;
        });
    }
    FC_CAPTURE_AND_RETHROW((o))
}
