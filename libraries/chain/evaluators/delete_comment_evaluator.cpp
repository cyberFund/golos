#include <steemit/chain/evaluators/delete_comment_evaluator.hpp>

void steemit::chain::delete_comment_evaluator::do_apply(const protocol::delete_comment_operation &o) {

    if (this->_db.has_hardfork(STEEMIT_HARDFORK_0_10)) {
        const auto &auth = this->_db.get_account(o.author);
        FC_ASSERT(!(auth.owner_challenged || auth.active_challenged),
                  "Operation cannot be processed because account is currently challenged.");
    }

    const auto &comment = this->_db.get_comment(o.author, o.permlink);
    FC_ASSERT(comment.children == 0, "Cannot delete a comment with replies.");

    if (this->_db.is_producing()) {
        FC_ASSERT(comment.net_rshares <= 0, "Cannot delete a comment with net positive votes.");
    }
    if (comment.net_rshares > 0) {
        return;
    }

    const auto &vote_idx = this->_db.get_index<comment_vote_index>().indices().get<by_comment_voter>();

    auto vote_itr = vote_idx.lower_bound(comment_id_type(comment.id));
    while (vote_itr != vote_idx.end() && vote_itr->comment == comment.id) {
        const auto &cur_vote = *vote_itr;
        ++vote_itr;
        this->_db.remove(cur_vote);
    }

    /// this loop can be skiped for validate-only nodes as it is merely gathering stats for indicies
    if (this->_db.has_hardfork(STEEMIT_HARDFORK_0_6__80) && comment.parent_author != STEEMIT_ROOT_POST_PARENT) {
        auto parent = &this->_db.get_comment(comment.parent_author, comment.parent_permlink);
        auto now = this->_db.head_block_time();
        while (parent) {
            this->_db.modify(*parent, [&](comment_object &p) {
                p.children--;
                p.active = now;
            });
#ifndef IS_LOW_MEM
            if (parent->parent_author != STEEMIT_ROOT_POST_PARENT) {
                parent = &this->_db.get_comment(parent->parent_author, parent->parent_permlink);
            } else
#endif
            {
                parent = nullptr;
            }
        }
    }

    this->_db.remove(comment);
}
