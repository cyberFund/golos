#include <steemit/chain/evaluators/account_witness_vote_evaluator.hpp>
void steemit::chain::account_witness_vote_evaluator::do_apply(const protocol::account_witness_vote_operation &o) {

    const auto &voter = this->_db.get_account(o.account);
    FC_ASSERT(voter.proxy.size() == 0, "A proxy is currently set, please clear the proxy before voting for a witness.");

    if (o.approve) {
        FC_ASSERT(voter.can_vote, "Account has declined its voting rights.");
    }

    const auto &witness = this->_db.get_witness(o.witness);

    const auto &by_account_witness_idx = this->_db.get_index<witness_vote_index>().indices().get<by_account_witness>();
    auto itr = by_account_witness_idx.find(boost::make_tuple(voter.id, witness.id));

    if (itr == by_account_witness_idx.end()) {
        FC_ASSERT(o.approve, "Vote doesn't exist, user must indicate a desire to approve witness.");

        if (this->_db.has_hardfork(STEEMIT_HARDFORK_0_2)) {
            FC_ASSERT(voter.witnesses_voted_for <
                      STEEMIT_MAX_ACCOUNT_WITNESS_VOTES,
                      "Account has voted for too many witnesses."); // TODO: Remove after hardfork 2

            this->_db.create<witness_vote_object>([&](witness_vote_object &v) {
                v.witness = witness.id;
                v.account = voter.id;
            });

            if (this->_db.has_hardfork(STEEMIT_HARDFORK_0_3)) {
                this->_db.adjust_witness_vote(witness, voter.witness_vote_weight());
            } else {
                this->_db.adjust_proxied_witness_votes(voter, voter.witness_vote_weight());
            }

        } else {

            this->_db.create<witness_vote_object>([&](witness_vote_object &v) {
                v.witness = witness.id;
                v.account = voter.id;
            });
            this->_db.modify(witness, [&](witness_object &w) {
                w.votes += voter.witness_vote_weight();
            });

        }
        this->_db.modify(voter, [&](account_object &a) {
            a.witnesses_voted_for++;
        });

    } else {
        FC_ASSERT(!o.approve, "Vote currently exists, user must indicate a desire to reject witness.");

        if (this->_db.has_hardfork(STEEMIT_HARDFORK_0_2)) {
            if (this->_db.has_hardfork(STEEMIT_HARDFORK_0_3)) {
                this->_db.adjust_witness_vote(witness, -voter.witness_vote_weight());
            } else {
                this->_db.adjust_proxied_witness_votes(voter, -voter.witness_vote_weight());
            }
        } else {
            this->_db.modify(witness, [&](witness_object &w) {
                w.votes -= voter.witness_vote_weight();
            });
        }
        this->_db.modify(voter, [&](account_object &a) {
            a.witnesses_voted_for--;
        });
        this->_db.remove(*itr);
    }
}
