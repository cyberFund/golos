#include <steemit/chain/evaluators/comment_evaluator.hpp>
void steemit::chain::comment_evaluator::do_apply(const protocol::comment_operation &o) {
    try {
        if (this->_db.is_producing() ||
            this->_db.has_hardfork(STEEMIT_HARDFORK_0_5__55)) {
            FC_ASSERT(o.title.size() + o.body.size() +
                      o.json_metadata.size(),
                      "Cannot update comment because nothing appears to be changing.");
        }

        const auto &by_permlink_idx = this->_db.get_index<comment_index>().indices().get<by_permlink>();
        auto itr = by_permlink_idx.find(boost::make_tuple(o.author, o.permlink));

        const auto &auth = this->_db.get_account(o.author); /// prove it exists

        if (this->_db.has_hardfork(STEEMIT_HARDFORK_0_10)) {
            FC_ASSERT(!(auth.owner_challenged ||
                        auth.active_challenged),
                      "Operation cannot be processed because account is currently challenged.");
        }

        comment_id_type id;

        const comment_object *parent = nullptr;
        if (o.parent_author != STEEMIT_ROOT_POST_PARENT) {
            parent = &this->_db.get_comment(o.parent_author, o.parent_permlink);
            if (!this->_db.has_hardfork(STEEMIT_HARDFORK_0_17__84)) {
                FC_ASSERT(parent->depth <
                          STEEMIT_MAX_COMMENT_DEPTH_PRE_HF17,
                          "Comment is nested ${x} posts deep, maximum depth is ${y}.",
                          ("x", parent->depth)("y", STEEMIT_MAX_COMMENT_DEPTH_PRE_HF17));
            } else if (this->_db.is_producing()) {
                FC_ASSERT(parent->depth <
                          STEEMIT_SOFT_MAX_COMMENT_DEPTH,
                          "Comment is nested ${x} posts deep, maximum depth is ${y}.",
                          ("x", parent->depth)("y", STEEMIT_SOFT_MAX_COMMENT_DEPTH));
            } else {
                FC_ASSERT(parent->depth <
                          STEEMIT_MAX_COMMENT_DEPTH,
                          "Comment is nested ${x} posts deep, maximum depth is ${y}.",
                          ("x", parent->depth)("y", STEEMIT_MAX_COMMENT_DEPTH));
            }

        }

        if ((this->_db.is_producing() ||
             this->_db.has_hardfork(STEEMIT_HARDFORK_0_17)) &&
            o.json_metadata.size())
            FC_ASSERT(fc::is_utf8(o.json_metadata), "JSON Metadata must be UTF-8");

        auto now = this->_db.head_block_time();

        if (itr == by_permlink_idx.end()) {
            if (o.parent_author != STEEMIT_ROOT_POST_PARENT) {
                FC_ASSERT(this->_db.get(parent->root_comment).allow_replies,
                          "The parent comment has disabled replies.");
                if (this->_db.has_hardfork(STEEMIT_HARDFORK_0_12__177) &&
                    !this->_db.has_hardfork(STEEMIT_HARDFORK_0_17__97)) {
                    FC_ASSERT(
                            this->_db.calculate_discussion_payout_time(*parent) !=
                            fc::time_point_sec::maximum(), "Discussion is frozen.");
                }
            }

            auto band = this->_db.find<account_bandwidth_object, by_account_bandwidth_type>(
                    boost::make_tuple(o.author, bandwidth_type::post));

            if (band == nullptr) {
                band = &this->_db.create<account_bandwidth_object>([&](account_bandwidth_object &b) {
                    b.account = o.author;
                    b.type = bandwidth_type::post;
                });
            }

            if (this->_db.has_hardfork(STEEMIT_HARDFORK_0_12__176)) {
                if (o.parent_author == STEEMIT_ROOT_POST_PARENT) {
                    FC_ASSERT((now - band->last_bandwidth_update) >
                              STEEMIT_MIN_ROOT_COMMENT_INTERVAL, "You may only post once every 5 minutes.",
                              ("now", now)("last_root_post", band->last_bandwidth_update));
                } else {
                    FC_ASSERT((now - auth.last_post) >
                              STEEMIT_MIN_REPLY_INTERVAL, "You may only comment once every 20 seconds.",
                              ("now", now)("auth.last_post", auth.last_post));
                }
            } else if (this->_db.has_hardfork(STEEMIT_HARDFORK_0_6__113)) {
                if (o.parent_author == STEEMIT_ROOT_POST_PARENT) {
                    FC_ASSERT((now - auth.last_post) >
                              STEEMIT_MIN_ROOT_COMMENT_INTERVAL, "You may only post once every 5 minutes.",
                              ("now", now)("auth.last_post", auth.last_post));
                } else {
                    FC_ASSERT((now - auth.last_post) >
                              STEEMIT_MIN_REPLY_INTERVAL, "You may only comment once every 20 seconds.",
                              ("now", now)("auth.last_post", auth.last_post));
                }
            } else {
                FC_ASSERT((now - auth.last_post) >
                          fc::seconds(60), "You may only post once per minute.",
                          ("now", now)("auth.last_post", auth.last_post));
            }

            uint16_t reward_weight = STEEMIT_100_PERCENT;

            if (o.parent_author == STEEMIT_ROOT_POST_PARENT) {
                auto post_bandwidth = band->average_bandwidth;

                if (this->_db.has_hardfork(STEEMIT_HARDFORK_0_17__78)) {
                    auto post_delta_time = std::min(
                            now.sec_since_epoch() -
                            band->last_bandwidth_update.sec_since_epoch(), STEEMIT_POST_AVERAGE_WINDOW);
                    auto old_weight = (post_bandwidth *
                                       (STEEMIT_POST_AVERAGE_WINDOW -
                                        post_delta_time)) /
                                      STEEMIT_POST_AVERAGE_WINDOW;
                    post_bandwidth = (old_weight + STEEMIT_100_PERCENT);
                    reward_weight = uint16_t(std::min(
                            (STEEMIT_POST_WEIGHT_CONSTANT *
                             STEEMIT_100_PERCENT) /
                            (post_bandwidth.value *
                             post_bandwidth.value), uint64_t(STEEMIT_100_PERCENT)));
                } else if (this->_db.has_hardfork(STEEMIT_HARDFORK_0_12__176)) {
                    auto post_delta_time = std::min(
                            now.sec_since_epoch() -
                            band->last_bandwidth_update.sec_since_epoch(), STEEMIT_POST_AVERAGE_WINDOW);
                    auto old_weight = (post_bandwidth *
                                       (STEEMIT_POST_AVERAGE_WINDOW -
                                        post_delta_time)) /
                                      STEEMIT_POST_AVERAGE_WINDOW;
                    post_bandwidth = (old_weight + STEEMIT_100_PERCENT);
                    reward_weight = uint16_t(std::min(
                            (STEEMIT_POST_WEIGHT_CONSTANT_PRE_HF17 *
                             STEEMIT_100_PERCENT) /
                            (post_bandwidth.value *
                             post_bandwidth.value), uint64_t(STEEMIT_100_PERCENT)));
                }

                this->_db.modify(*band, [&](account_bandwidth_object &b) {
                    b.last_bandwidth_update = now;
                    b.average_bandwidth = post_bandwidth;
                });
            }

            db().modify(auth, [&](account_object &a) {
                a.last_post = now;
                a.post_count++;
            });

            const auto &new_comment = this->_db.create<comment_object>([&](comment_object &com) {
                if (this->_db.has_hardfork(STEEMIT_HARDFORK_0_1)) {
                    validate_permlink_0_1(o.parent_permlink);
                    validate_permlink_0_1(o.permlink);
                }

                com.author = o.author;
                from_string(com.permlink, o.permlink);
                com.last_update = this->_db.head_block_time();
                com.created = com.last_update;
                com.active = com.last_update;
                com.last_payout = fc::time_point_sec::min();
                com.max_cashout_time = fc::time_point_sec::maximum();
                com.reward_weight = reward_weight;

                if (o.parent_author == STEEMIT_ROOT_POST_PARENT) {
                    com.parent_author = "";
                    from_string(com.parent_permlink, o.parent_permlink);
                    com.root_comment = com.id;
                    com.cashout_time = this->_db.has_hardfork(STEEMIT_HARDFORK_0_12__177)
                                       ?
                                       this->_db.head_block_time() +
                                       STEEMIT_CASHOUT_WINDOW_SECONDS_PRE_HF17
                                       :
                                       fc::time_point_sec::maximum();
                } else {
                    com.parent_author = parent->author;
                    com.parent_permlink = parent->permlink;
                    com.depth = parent->depth + 1;
                    com.root_comment = parent->root_comment;
                    com.cashout_time = fc::time_point_sec::maximum();
                }

                if (this->_db.has_hardfork(STEEMIT_HARDFORK_0_17__91)) {
                    com.cashout_time = com.created +
                                       STEEMIT_CASHOUT_WINDOW_SECONDS;
                }

#ifndef IS_LOW_MEM
                from_string(com.title, o.title);
                if (o.body.size() < 1024 * 1024 * 128) {
                    from_string(com.body, o.body);
                }
                if (fc::is_utf8(o.json_metadata)) {
                    from_string(com.json_metadata, o.json_metadata);
                } else
                    wlog("Comment ${a}/${p} contains invalid UTF-8 metadata",
                         ("a", o.author)("p", o.permlink));
#endif
            });


            id = new_comment.id;

/// this loop can be skiped for validate-only nodes as it is merely gathering stats for indicies
            auto now = this->_db.head_block_time();
            while (parent) {
                this->_db.modify(*parent, [&](comment_object &p) {
                    p.children++;
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

        } else // start edit case
        {
            const auto &comment = *itr;

            if (this->_db.has_hardfork(STEEMIT_HARDFORK_0_17__85)) {
                // This will be moved to the witness plugin in a later release
                if (this->_db.is_producing()) {
                    // For now, use the same editting rules, but implement it as a soft fork.
                    FC_ASSERT(
                            this->_db.calculate_discussion_payout_time(comment) !=
                            fc::time_point_sec::maximum(), "The comment is archived.");
                }
            } else if (this->_db.has_hardfork(STEEMIT_HARDFORK_0_14__306)) {
                FC_ASSERT(
                        this->_db.calculate_discussion_payout_time(comment) !=
                        fc::time_point_sec::maximum(), "The comment is archived.");
            } else if (this->_db.has_hardfork(STEEMIT_HARDFORK_0_10)) {
                FC_ASSERT(comment.last_payout ==
                          fc::time_point_sec::min(), "Can only edit during the first 24 hours.");
            }

            this->_db.modify(comment, [&](comment_object &com) {
                com.last_update = this->_db.head_block_time();
                com.active = com.last_update;
                strcmp_equal equal;

                if (!parent) {
                    FC_ASSERT(com.parent_author ==
                              account_name_type(), "The parent of a comment cannot change.");
                    FC_ASSERT(equal(com.parent_permlink, o.parent_permlink),
                              "The permlink of a comment cannot change.");
                } else {
                    FC_ASSERT(com.parent_author ==
                              o.parent_author, "The parent of a comment cannot change.");
                    FC_ASSERT(equal(com.parent_permlink, o.parent_permlink),
                              "The permlink of a comment cannot change.");
                }

#ifndef IS_LOW_MEM
                if (o.title.size()) {
                    from_string(com.title, o.title);
                }
                if (o.json_metadata.size()) {
                    if (fc::is_utf8(o.json_metadata)) {
                        from_string(com.json_metadata, o.json_metadata);
                    } else
                        wlog("Comment ${a}/${p} contains invalid UTF-8 metadata",
                             ("a", o.author)("p", o.permlink));
                }

                if (o.body.size()) {
                    try {
                        diff_match_patch <std::wstring> dmp;
                        auto patch = dmp.patch_fromText(utf8_to_wstring(o.body));
                        if (patch.size()) {
                            auto result = dmp.patch_apply(patch, utf8_to_wstring(to_string(com.body)));
                            auto patched_body = wstring_to_utf8(result.first);
                            if (!fc::is_utf8(patched_body)) {
                                idump(("invalid utf8")(patched_body));
                                from_string(com.body, fc::prune_invalid_utf8(patched_body));
                            } else {
                                from_string(com.body, patched_body);
                            }
                        } else { // replace
                            from_string(com.body, o.body);
                        }
                    } catch (...) {
                        from_string(com.body, o.body);
                    }
                }
#endif

            });

        } // end EDIT case

    }
    FC_CAPTURE_AND_RETHROW((o))
}
