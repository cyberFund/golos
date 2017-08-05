#include <steemit/chain/evaluators/comment_options_evaluator.hpp>
void steemit::chain::comment_options_evaluator::do_apply(const protocol::comment_options_operation &o) {

    if (this->_db.has_hardfork(STEEMIT_HARDFORK_0_10)) {
        const auto &auth = this->_db.get_account(o.author);
        FC_ASSERT(!(auth.owner_challenged ||
                    auth.active_challenged),
                  "Operation cannot be processed because account is currently challenged.");
    }

    const auto &comment = this->_db.get_comment(o.author, o.permlink);
    if (!o.allow_curation_rewards || !o.allow_votes ||
        o.max_accepted_payout < comment.max_accepted_payout) {
        FC_ASSERT(comment.abs_rshares ==
                  0,
                  "One of the included comment options requires the comment to have no rshares allocated to it.");
    }

    if (!this->_db.has_hardfork(STEEMIT_HARDFORK_0_17__102)) { // TODO: Remove after hardfork 17
        FC_ASSERT(o.extensions.size() ==
                  0, "Operation extensions for the comment_options_operation are not currently supported.");
    }

    FC_ASSERT(comment.allow_curation_rewards >=
              o.allow_curation_rewards, "Curation rewards cannot be re-enabled.");
    FC_ASSERT(comment.allow_votes >=
              o.allow_votes, "Voting cannot be re-enabled.");
    FC_ASSERT(comment.max_accepted_payout >=
              o.max_accepted_payout, "A comment cannot accept a greater payout.");
    FC_ASSERT(comment.percent_steem_dollars >=
              o.percent_steem_dollars, "A comment cannot accept a greater percent SBD.");

    this->_db.modify(comment, [&](comment_object &c) {
        c.max_accepted_payout = o.max_accepted_payout;
        c.percent_steem_dollars = o.percent_steem_dollars;
        c.allow_votes = o.allow_votes;
        c.allow_curation_rewards = o.allow_curation_rewards;
    });

    for (auto &e : o.extensions) {
        e.visit(comment_options_extension_visitor(comment, this->_db));
    }
}
