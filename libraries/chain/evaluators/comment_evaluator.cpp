#include <golos/chain/evaluators/comment_evaluator.hpp>
#include <golos/chain/database.hpp>
#include <golos/chain/database_exceptions.hpp>

#ifndef STEEMIT_BUILD_LOW_MEMORY

#include <diff_match_patch.h>
#include <boost/locale/encoding_utf.hpp>

using boost::locale::conv::utf_to_utf;

std::wstring utf8_to_wstring(const std::string &str) {
    return utf_to_utf<wchar_t>(str.c_str(), str.c_str() + str.size());
}

std::string wstring_to_utf8(const std::wstring &str) {
    return utf_to_utf<char>(str.c_str(), str.c_str() + str.size());
}

#endif

namespace golos {
    namespace chain {
        struct strcmp_equal {
            bool operator()(const shared_string &a, const std::string &b) {
                return a.size() == b.size() || std::strcmp(a.c_str(), b.c_str()) == 0;
            }
        };

        inline void validate_permlink_0_1(const std::string &permlink) {
            FC_ASSERT(permlink.size() > STEEMIT_MIN_PERMLINK_LENGTH && permlink.size() < STEEMIT_MAX_PERMLINK_LENGTH,
                      "Permlink is not a valid size.");

            FC_ASSERT(std::find_if_not(permlink.begin(), permlink.end(), [&](std::string::value_type c) -> bool {
                return std::isdigit(c) || std::islower(c) || c == '-';
            }) == permlink.end(), "Invalid permlink character");
        }

        /**
                 *  Because net_rshares is 0 there is no need to update any pending payout calculations or parent posts.
                 */
        template<uint8_t Major, uint8_t Hardfork, uint16_t Release>
        void delete_comment_evaluator<Major, Hardfork, Release>::do_apply(const operation_type &o) {

            if (this->db.has_hardfork(STEEMIT_HARDFORK_0_10)) {
                const auto &auth = this->db.get_account(o.author);
                FC_ASSERT(!(auth.owner_challenged || auth.active_challenged),
                          "Operation cannot be processed because account is currently challenged.");
            }

            const auto &comment = this->db.get_comment(o.author, o.permlink);
            FC_ASSERT(comment.children == 0, "Cannot delete a comment with replies.");

            if (this->db.is_producing()) {
                FC_ASSERT(comment.net_rshares <= 0, "Cannot delete a comment with net positive votes.");
            }
            if (comment.net_rshares > 0) {
                return;
            }

            const auto &vote_idx = this->db.template get_index<comment_vote_index>().indices().
                    template get<by_comment_voter>();

            auto vote_itr = vote_idx.lower_bound(comment_object::id_type(comment.id));
            while (vote_itr != vote_idx.end() && vote_itr->comment == comment.id) {
                const auto &cur_vote = *vote_itr;
                ++vote_itr;
                this->db.remove(cur_vote);
            }

            /// this loop can be skiped for validate-only nodes as it is merely gathering stats for indicies
            if (this->db.has_hardfork(STEEMIT_HARDFORK_0_6__80) && comment.parent_author != STEEMIT_ROOT_POST_PARENT) {
                auto parent = &this->db.get_comment(comment.parent_author, comment.parent_permlink);
                auto now = this->db.head_block_time();
                while (parent) {
                    this->db.modify(*parent, [&](comment_object &p) {
                        p.children--;
                        p.active = now;
                    });
#ifndef STEEMIT_BUILD_LOW_MEMORY
                    if (parent->parent_author != STEEMIT_ROOT_POST_PARENT) {
                        parent = &this->db.get_comment(parent->parent_author, parent->parent_permlink);
                    } else
#endif
                    {
                        parent = nullptr;
                    }
                }
            }

            /** TODO move category behavior to a plugin, this is not part of consensus */
            const category_object *cat = this->db.find_category(comment.category);
            this->db.modify(*cat, [&](category_object &c) {
                c.discussions--;
                c.last_update = this->db.head_block_time();
            });

            this->db.remove(comment);
        }

        struct comment_options_extension_visitor {
            comment_options_extension_visitor(const comment_object &c, database &input_db) : _c(c), db(input_db) {
            }

            typedef void result_type;

            const comment_object &_c;
            database &db;

            void operator()(const comment_payout_beneficiaries &cpb) const {
                if (this->db.is_producing()) {
                    FC_ASSERT(cpb.beneficiaries.size() <= 8, "Cannot specify more than 8 beneficiaries.");
                }

                FC_ASSERT(_c.beneficiaries.size() == 0, "Comment already has beneficiaries specified.");
                FC_ASSERT(_c.abs_rshares == 0, "Comment must not have been voted on before specifying beneficiaries.");

                this->db.modify(_c, [&](comment_object &c) {
                    for (auto &b : cpb.beneficiaries) {
                        auto acc = this->db.template find<account_object, by_name>(b.account);
                        FC_ASSERT(acc != nullptr, "Beneficiary \"${a}\" must exist.", ("a", b.account));
                        c.beneficiaries.push_back(b);
                    }
                });
            }
        };

        template<uint8_t Major, uint8_t Hardfork, uint16_t Release>
        void comment_options_evaluator<Major, Hardfork, Release>::do_apply(const operation_type &o) {

            if (this->db.has_hardfork(STEEMIT_HARDFORK_0_10)) {
                const auto &auth = this->db.get_account(o.author);
                FC_ASSERT(!(auth.owner_challenged || auth.active_challenged),
                          "Operation cannot be processed because account is currently challenged.");
            }

            const auto &comment = this->db.get_comment(o.author, o.permlink);
            if (!o.allow_curation_rewards || !o.allow_votes || o.max_accepted_payout < comment.max_accepted_payout) {
                FC_ASSERT(comment.abs_rshares == 0,
                          "One of the included comment options requires the comment to have no rshares allocated to it.");
            }

            if (!this->db.has_hardfork(STEEMIT_HARDFORK_0_17__102)) { // TODO: Remove after hardfork 17
                FC_ASSERT(o.extensions.size() == 0,
                          "Operation extensions for the comment_options_operation are not currently supported.");
            }

            FC_ASSERT(comment.allow_curation_rewards >= o.allow_curation_rewards,
                      "Curation rewards cannot be re-enabled.");
            FC_ASSERT(comment.allow_votes >= o.allow_votes, "Voting cannot be re-enabled.");
            FC_ASSERT(comment.max_accepted_payout >= o.max_accepted_payout,
                      "A comment cannot accept a greater payout.");
            FC_ASSERT(comment.percent_steem_dollars >= o.percent_steem_dollars,
                      "A comment cannot accept a greater percent SBD.");

            this->db.modify(comment, [&](comment_object &c) {
                c.max_accepted_payout = asset<0, 17, 0>(o.max_accepted_payout.amount,
                                                        o.max_accepted_payout.symbol_name());
                c.percent_steem_dollars = o.percent_steem_dollars;
                c.allow_votes = o.allow_votes;
                c.allow_curation_rewards = o.allow_curation_rewards;
            });

            for (auto &e : o.extensions) {
                e.visit(comment_options_extension_visitor(comment, this->db));
            }
        }

        template<uint8_t Major, uint8_t Hardfork, uint16_t Release>
        void comment_evaluator<Major, Hardfork, Release>::do_apply(const operation_type &o) {
            try {
                if (this->db.is_producing() || this->db.has_hardfork(STEEMIT_HARDFORK_0_5__55)) {
                    FC_ASSERT(o.title.size() + o.body.size() + o.json_metadata.size(),
                              "Cannot update comment because nothing appears to be changing.");
                }

                const auto &by_permlink_idx = this->db.template get_index<comment_index>().indices().
                        template get<by_permlink>();
                auto itr = by_permlink_idx.find(boost::make_tuple(o.author, o.permlink));

                const auto &auth = this->db.get_account(o.author); /// prove it exists

                if (this->db.has_hardfork(STEEMIT_HARDFORK_0_10)) {
                    FC_ASSERT(!(auth.owner_challenged || auth.active_challenged),
                              "Operation cannot be processed because account is currently challenged.");
                }

                comment_object::id_type id;

                const comment_object *parent = nullptr;
                if (o.parent_author != STEEMIT_ROOT_POST_PARENT) {
                    parent = &this->db.get_comment(o.parent_author, o.parent_permlink);
                    if (!this->db.has_hardfork(STEEMIT_HARDFORK_0_17__84)) {
                        FC_ASSERT(parent->depth < STEEMIT_MAX_COMMENT_DEPTH_PRE_HF17,
                                  "Comment is nested ${x} posts deep, maximum depth is ${y}.",
                                  ("x", parent->depth)("y", STEEMIT_MAX_COMMENT_DEPTH_PRE_HF17));
                    } else if (this->db.is_producing()) {
                        FC_ASSERT(parent->depth < STEEMIT_SOFT_MAX_COMMENT_DEPTH,
                                  "Comment is nested ${x} posts deep, maximum depth is ${y}.",
                                  ("x", parent->depth)("y", STEEMIT_SOFT_MAX_COMMENT_DEPTH));
                    } else {
                        FC_ASSERT(parent->depth < STEEMIT_MAX_COMMENT_DEPTH,
                                  "Comment is nested ${x} posts deep, maximum depth is ${y}.",
                                  ("x", parent->depth)("y", STEEMIT_MAX_COMMENT_DEPTH));
                    }

                }

                if ((this->db.is_producing() || this->db.has_hardfork(STEEMIT_HARDFORK_0_17)) &&
                    o.json_metadata.size()) {
                    FC_ASSERT(fc::is_utf8(o.json_metadata), "JSON Metadata must be UTF-8");
                }

                auto now = this->db.head_block_time();

                if (itr == by_permlink_idx.end()) {
                    if (o.parent_author != STEEMIT_ROOT_POST_PARENT) {
                        FC_ASSERT(this->db.get(parent->root_comment).allow_replies,
                                  "The parent comment has disabled replies.");
                        if (this->db.has_hardfork(STEEMIT_HARDFORK_0_12__177) &&
                            !this->db.has_hardfork(STEEMIT_HARDFORK_0_17__97)) {
                            FC_ASSERT(
                                    this->db.calculate_discussion_payout_time(*parent) != fc::time_point_sec::maximum(),
                                    "Discussion is frozen.");
                        }
                    }

                    auto band = this->db.template find<account_bandwidth_object, by_account_bandwidth_type>(
                            boost::make_tuple(o.author, bandwidth_type::post));

                    if (band == nullptr) {
                        band = &this->db.template create<account_bandwidth_object>([&](account_bandwidth_object &b) {
                            b.account = o.author;
                            b.type = bandwidth_type::post;
                        });
                    }

                    if (this->db.has_hardfork(STEEMIT_HARDFORK_0_12__176)) {
                        if (o.parent_author == STEEMIT_ROOT_POST_PARENT) {
                            FC_ASSERT((now - band->last_bandwidth_update) > STEEMIT_MIN_ROOT_COMMENT_INTERVAL,
                                      "You may only post once every 5 minutes.",
                                      ("now", now)("last_root_post", band->last_bandwidth_update));
                        } else {
                            FC_ASSERT((now - auth.last_post) > STEEMIT_MIN_REPLY_INTERVAL,
                                      "You may only comment once every 20 seconds.",
                                      ("now", now)("auth.last_post", auth.last_post));
                        }
                    } else if (this->db.has_hardfork(STEEMIT_HARDFORK_0_6__113)) {
                        if (o.parent_author == STEEMIT_ROOT_POST_PARENT) {
                            FC_ASSERT((now - auth.last_post) > STEEMIT_MIN_ROOT_COMMENT_INTERVAL,
                                      "You may only post once every 5 minutes.",
                                      ("now", now)("auth.last_post", auth.last_post));
                        } else {
                            FC_ASSERT((now - auth.last_post) > STEEMIT_MIN_REPLY_INTERVAL,
                                      "You may only comment once every 20 seconds.",
                                      ("now", now)("auth.last_post", auth.last_post));
                        }
                    } else {
                        FC_ASSERT((now - auth.last_post) > fc::seconds(60), "You may only post once per minute.",
                                  ("now", now)("auth.last_post", auth.last_post));
                    }

                    uint16_t reward_weight = STEEMIT_100_PERCENT;

                    if (o.parent_author == STEEMIT_ROOT_POST_PARENT) {
                        auto post_bandwidth = band->average_bandwidth;

                        if (this->db.has_hardfork(STEEMIT_HARDFORK_0_17__78)) {
                            auto post_delta_time = std::min(
                                    now.sec_since_epoch() - band->last_bandwidth_update.sec_since_epoch(),
                                    STEEMIT_POST_AVERAGE_WINDOW);
                            auto old_weight = (post_bandwidth * (STEEMIT_POST_AVERAGE_WINDOW - post_delta_time)) /
                                              STEEMIT_POST_AVERAGE_WINDOW;
                            post_bandwidth = (old_weight + STEEMIT_100_PERCENT);
                            reward_weight = uint16_t(std::min((STEEMIT_POST_WEIGHT_CONSTANT * STEEMIT_100_PERCENT) /
                                                              (post_bandwidth.value * post_bandwidth.value),
                                                              uint64_t(STEEMIT_100_PERCENT)));
                        } else if (this->db.has_hardfork(STEEMIT_HARDFORK_0_12__176)) {
                            auto post_delta_time = std::min(
                                    now.sec_since_epoch() - band->last_bandwidth_update.sec_since_epoch(),
                                    STEEMIT_POST_AVERAGE_WINDOW);
                            auto old_weight = (post_bandwidth * (STEEMIT_POST_AVERAGE_WINDOW - post_delta_time)) /
                                              STEEMIT_POST_AVERAGE_WINDOW;
                            post_bandwidth = (old_weight + STEEMIT_100_PERCENT);
                            reward_weight = uint16_t(std::min(
                                    (STEEMIT_POST_WEIGHT_CONSTANT_PRE_HF17 * STEEMIT_100_PERCENT) /
                                    (post_bandwidth.value * post_bandwidth.value), uint64_t(STEEMIT_100_PERCENT)));
                        }

                        this->db.modify(*band, [&](account_bandwidth_object &b) {
                            b.last_bandwidth_update = now;
                            b.average_bandwidth = post_bandwidth;
                        });
                    }

                    this->db.modify(auth, [&](account_object &a) {
                        a.last_post = now;
                        a.post_count++;
                    });

                    const auto &new_comment = this->db.template create<comment_object>([&](comment_object &com) {
                        if (this->db.has_hardfork(STEEMIT_HARDFORK_0_1)) {
                            validate_permlink_0_1(o.parent_permlink);
                            validate_permlink_0_1(o.permlink);
                        }

                        com.author = o.author;
                        from_string(com.permlink, o.permlink);
                        com.last_update = this->db.head_block_time();
                        com.created = com.last_update;
                        com.active = com.last_update;
                        com.last_payout = fc::time_point_sec::min();
                        com.max_cashout_time = fc::time_point_sec::maximum();
                        com.reward_weight = reward_weight;

                        if (o.parent_author == STEEMIT_ROOT_POST_PARENT) {
                            com.parent_author = "";
                            from_string(com.parent_permlink, o.parent_permlink);
                            from_string(com.category, o.parent_permlink);
                            com.root_comment = com.id;
                            com.cashout_time = this->db.has_hardfork(STEEMIT_HARDFORK_0_12__177) ?
                                               this->db.head_block_time() + STEEMIT_CASHOUT_WINDOW_SECONDS_PRE_HF17
                                                                                                 : fc::time_point_sec::maximum();
                        } else {
                            com.parent_author = parent->author;
                            com.parent_permlink = parent->permlink;
                            com.depth = parent->depth + 1;
                            com.category = parent->category;
                            com.root_comment = parent->root_comment;
                            com.cashout_time = fc::time_point_sec::maximum();
                        }

                        if (this->db.has_hardfork(STEEMIT_HARDFORK_0_17__91)) {
                            com.cashout_time = com.created + STEEMIT_CASHOUT_WINDOW_SECONDS;
                        }

#ifndef STEEMIT_BUILD_LOW_MEMORY
                        from_string(com.title, o.title);
                        if (o.body.size() < 1024 * 1024 * 128) {
                            from_string(com.body, o.body);
                        }
                        if (fc::is_utf8(o.json_metadata)) {
                            from_string(com.json_metadata, o.json_metadata);
                        } else {
                            wlog("Comment ${a}/${p} contains invalid UTF-8 metadata", ("a", o.author)("p", o.permlink));
                        }
#endif
                    });

                    /** TODO move category behavior to a plugin, this is not part of consensus */
                    const category_object *cat = this->db.find_category(new_comment.category);
                    if (!cat) {
                        cat = &this->db.template create<category_object>([&](category_object &c) {
                            c.name = new_comment.category;
                            c.discussions = 1;
                            c.last_update = this->db.head_block_time();
                        });
                    } else {
                        this->db.modify(*cat, [&](category_object &c) {
                            c.discussions++;
                            c.last_update = this->db.head_block_time();
                        });
                    }

                    id = new_comment.id;

                    /// this loop can be skiped for validate-only nodes as it is merely gathering stats for indicies
                    auto now = this->db.head_block_time();
                    while (parent) {
                        this->db.modify(*parent, [&](comment_object &p) {
                            p.children++;
                            p.active = now;
                        });
#ifndef STEEMIT_BUILD_LOW_MEMORY
                        if (parent->parent_author != STEEMIT_ROOT_POST_PARENT) {
                            parent = &this->db.get_comment(parent->parent_author, parent->parent_permlink);
                        } else
#endif
                        {
                            parent = nullptr;
                        }
                    }

                } else // start edit case
                {
                    const auto &comment = *itr;

                    if (this->db.has_hardfork(STEEMIT_HARDFORK_0_17__85)) {
                        // This will be moved to the witness plugin in a later release
                        if (this->db.is_producing()) {
                            // For now, use the same editting rules, but implement it as a soft fork.
                            FC_ASSERT(
                                    this->db.calculate_discussion_payout_time(comment) != fc::time_point_sec::maximum(),
                                    "The comment is archived.");
                        }
                    } else if (this->db.has_hardfork(STEEMIT_HARDFORK_0_14__306)) {
                        FC_ASSERT(this->db.calculate_discussion_payout_time(comment) != fc::time_point_sec::maximum(),
                                  "The comment is archived.");
                    } else if (this->db.has_hardfork(STEEMIT_HARDFORK_0_10)) {
                        FC_ASSERT(comment.last_payout == fc::time_point_sec::min(),
                                  "Can only edit during the first 24 hours.");
                    }

                    this->db.modify(comment, [&](comment_object &com) {
                        com.last_update = this->db.head_block_time();
                        com.active = com.last_update;
                        strcmp_equal equal;

                        if (!parent) {
                            FC_ASSERT(com.parent_author == account_name_type(),
                                      "The parent of a comment cannot change.");
                            FC_ASSERT(equal(com.parent_permlink, o.parent_permlink),
                                      "The permlink of a comment cannot change.");
                        } else {
                            FC_ASSERT(com.parent_author == o.parent_author, "The parent of a comment cannot change.");
                            FC_ASSERT(equal(com.parent_permlink, o.parent_permlink),
                                      "The permlink of a comment cannot change.");
                        }

#ifndef STEEMIT_BUILD_LOW_MEMORY
                        if (o.title.size()) {
                            from_string(com.title, o.title);
                        }
                        if (o.json_metadata.size()) {
                            if (fc::is_utf8(o.json_metadata)) {
                                from_string(com.json_metadata, o.json_metadata);
                            } else {
                                wlog("Comment ${a}/${p} contains invalid UTF-8 metadata",
                                     ("a", o.author)("p", o.permlink));
                            }
                        }

                        if (o.body.size()) {
                            try {
                                diff_match_patch<std::wstring> dmp;
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

            } FC_CAPTURE_AND_RETHROW((o))
        }
    }
}

#include <golos/chain/evaluators/comment_evaluator.tpp>