#ifndef GOLOS_COMMENT_OBJECT_POLICY_HPP
#define GOLOS_COMMENT_OBJECT_POLICY_HPP
namespace steemit {
namespace chain {
class comment_policy {
    comment_policy() = default;

    comment_policy(const comment_policy &) = default;

    comment_policy &operator=(const comment_policy &) = default;

    comment_policy(comment_policy &&) = default;

    comment_policy &operator=(comment_policy &&) = default;

    virtual ~comment_policy() = default;

    comment_policy(database_basic &ref, evaluator_registry <operation> &evaluator_registry_)
            : references(ref) {
            evaluator_registry_.register_evaluator<comment_evaluator>();
            evaluator_registry_.register_evaluator<comment_options_evaluator>();
            evaluator_registry_.register_evaluator<delete_comment_evaluator>();
    }

    const time_point_sec calculate_discussion_payout_time(const comment_object &comment) const {
        if (has_hardfork(STEEMIT_HARDFORK_0_17__91) ||
            comment.parent_author == STEEMIT_ROOT_POST_PARENT) {
            return comment.cashout_time;
        } else {
            return get<comment_object>(comment.root_comment).cashout_time;
        }
    }

    const reward_fund_object &get_reward_fund(const comment_object &c) const {
        return get<reward_fund_object, by_name>(
                c.parent_author == STEEMIT_ROOT_POST_PARENT
                ? STEEMIT_POST_REWARD_FUND_NAME
                : STEEMIT_COMMENT_REWARD_FUND_NAME);
    }


    void adjust_total_payout(const comment_object &cur, const asset &sbd_created, const asset &curator_sbd_value, const asset &beneficiary_value) {
        modify(cur, [&](comment_object &c) {
            if (c.total_payout_value.symbol == sbd_created.symbol) {
                c.total_payout_value += sbd_created;
            }
            c.beneficiary_payout_value += beneficiary_value;
            c.curator_payout_value += curator_sbd_value;
        });
        /// TODO: potentially modify author's total payout numbers as well
    }

/**
 *  This method will iterate through all comment_vote_objects and give them
 *  (max_rewards * weight) / c.total_vote_weight.
 *
 *  @returns unclaimed rewards.
 */
    share_type pay_curators(const comment_object &c, share_type &max_rewards) {
        try {
            uint128_t total_weight(c.total_vote_weight);
            //edump( (total_weight)(max_rewards) );
            share_type unclaimed_rewards = max_rewards;

            if (!c.allow_curation_rewards) {
                unclaimed_rewards = 0;
                max_rewards = 0;
            } else if (c.total_vote_weight > 0) {
                const auto &cvidx = get_index<comment_vote_index>().indices().get<by_comment_weight_voter>();
                auto itr = cvidx.lower_bound(c.id);
                while (itr != cvidx.end() && itr->comment == c.id) {
                    uint128_t weight(itr->weight);
                    auto claim = ((max_rewards.value * weight) /
                                  total_weight).to_uint64();
                    if (claim > 0) // min_amt is non-zero satoshis
                    {
                        unclaimed_rewards -= claim;
                        const auto &voter = get(itr->voter);
                        auto reward = create_vesting(voter, asset(claim, STEEM_SYMBOL));

                        push_virtual_operation(curation_reward_operation(voter.name, reward, c.author, to_string(c.permlink)));

#ifndef STEEMIT_BUILD_LOW_MEMORY
                        modify(voter, [&](account_object &a) {
                            a.curation_rewards += claim;
                        });
#endif
                    }
                    ++itr;
                }
            }

            max_rewards -= unclaimed_rewards;

            return unclaimed_rewards;
        } FC_CAPTURE_AND_RETHROW()
    }

    void fill_comment_reward_context_local_state(utilities::comment_reward_context &ctx, const comment_object &comment) {
        ctx.rshares = comment.net_rshares;
        ctx.reward_weight = comment.reward_weight;
        ctx.max_sbd = comment.max_accepted_payout;
    }


    void update_children_rshares2(database_basic &db, const comment_object &c, const fc::uint128_t &old_rshares2, const fc::uint128_t &new_rshares2) {
        // Iteratively updates the children_rshares2 of this comment and all of its ancestors

        const comment_object *current_comment = &c;
        while (true) {
            db.modify(*current_comment, [&](comment_object &comment) {
                comment.children_rshares2 -= old_rshares2;
                comment.children_rshares2 += new_rshares2;
            });

            if (current_comment->depth == 0) {
                break;
            }

            current_comment = &db.get_comment(current_comment->parent_author, current_comment->parent_permlink);
        }
    }

    /** This method updates total_reward_shares2 on DGPO, and children_rshares2 on comments, when a comment's rshares2 changes
    * from old_rshares2 to new_rshares2.  Maintaining invariants that children_rshares2 is the sum of all descendants' rshares2,
    * and dgpo.total_reward_shares2 is the total number of rshares2 outstanding.
    */

    void adjust_rshares2(const comment_object &c, fc::uint128_t old_rshares2, fc::uint128_t new_rshares2) {
        update_children_rshares2(*this, c, old_rshares2, new_rshares2);

        const auto &dgpo = get_dynamic_global_properties();
        modify(dgpo, [&](dynamic_global_property_object &p) {
            p.total_reward_shares2 -= old_rshares2;
            p.total_reward_shares2 += new_rshares2;
        });
    }

    void retally_comment_children() {
            const auto &cidx = get_index<comment_index>().indices();

            // Clear children counts
            for (auto itr = cidx.begin(); itr != cidx.end(); ++itr) {
                    modify(*itr, [&](comment_object &c) {
                        c.children = 0;
                    });
            }

            for (auto itr = cidx.begin(); itr != cidx.end(); ++itr) {
                    if (itr->parent_author != STEEMIT_ROOT_POST_PARENT) {
// Low memory nodes only need immediate child count, full nodes track total children
#ifdef STEEMIT_BUILD_LOW_MEMORY
                            modify(get_comment(itr->parent_author, itr->parent_permlink), [&](comment_object &c) {
                        c.children++;
                    });
#else
                            const comment_object *parent = &get_comment(itr->parent_author, itr->parent_permlink);
                            while (parent) {
                                    modify(*parent, [&](comment_object &c) {
                                        c.children++;
                                    });

                                    if (parent->parent_author != STEEMIT_ROOT_POST_PARENT) {
                                            parent = &get_comment(parent->parent_author, parent->parent_permlink);
                                    } else {
                                            parent = nullptr;
                                    }
                            }
#endif
                    }
            }
    }

    void process_comment_cashout() {
            /// don't allow any content to get paid out until the website is ready to launch
            /// and people have had a week to start posting.  The first cashout will be the biggest because it
            /// will represent 2+ months of rewards.
//            if (!has_hardfork(STEEMIT_FIRST_CASHOUT_TIME)) {
//                return;
//            }

            if (head_block_time() <= STEEMIT_FIRST_CASHOUT_TIME) {
                    return;
            }

            const auto &gpo = get_dynamic_global_properties();
            utilities::comment_reward_context ctx;

            ctx.current_steem_price = get_feed_history().current_median_history;

            vector<reward_fund_context> funds;
            vector<share_type> steem_awarded;
            const auto &reward_idx = get_index<reward_fund_index, by_id>();

            for (auto itr = reward_idx.begin();
                 itr != reward_idx.end(); ++itr) {
                    // Add all reward funds to the local cache and decay their recent rshares
                    modify(*itr, [&](reward_fund_object &rfo) {
                        rfo.recent_rshares2 -= (rfo.recent_rshares2 *
                                                (head_block_time() -
                                                 rfo.last_update).to_seconds()) /
                                               STEEMIT_RECENT_RSHARES_DECAY_RATE.to_seconds();
                        rfo.last_update = head_block_time();
                    });

                    reward_fund_context rf_ctx;
                    rf_ctx.recent_rshares2 = itr->recent_rshares2;
                    rf_ctx.reward_balance = itr->reward_balance;

                    funds.push_back(rf_ctx);
            }

            const auto &cidx = get_index<comment_index>().indices().get<by_cashout_time>();
            const auto &com_by_root = get_index<comment_index>().indices().get<by_root>();

            auto current = cidx.begin();
            //  add all rshares about to be cashed out to the reward funds
            if (has_hardfork(STEEMIT_HARDFORK_0_17__89)) {
                    while (current != cidx.end() &&
                           current->cashout_time <= head_block_time()) {
                            if (current->net_rshares > 0) {
                                    const auto &rf = get_reward_fund(*current);
                                    funds[rf.id._id].recent_rshares2 += utilities::calculate_vshares(current->net_rshares.value, rf);
                                    FC_ASSERT(funds[rf.id._id].recent_rshares2 <
                                              std::numeric_limits<uint64_t>::max());
                            }

                            ++current;
                    }

                    current = cidx.begin();
            }

            /*
             * Payout all comments
             *
             * Each payout follows a similar pattern, but for a different reason.
             * Cashout comment helper does not know about the reward fund it is paying from.
             * The helper only does token allocation based on curation rewards and the SBD
             * global %, etc.
             *
             * Each context is used by get_rshare_reward to determine what part of each budget
             * the comment is entitled to. Prior to hardfork 17, all payouts are done against
             * the global state updated each payout. After the hardfork, each payout is done
             * against a reward fund state that is snapshotted before all payouts in the block.
             */

            while (current != cidx.end() &&
                   current->cashout_time <= head_block_time()) {
                    if (has_hardfork(STEEMIT_HARDFORK_0_17__89)) {
                            auto fund_id = get_reward_fund(*current).id._id;
                            ctx.total_reward_shares2 = funds[fund_id].recent_rshares2;
                            ctx.total_reward_fund_steem = funds[fund_id].reward_balance;
                            funds[fund_id].steem_awarded += cashout_comment_helper(ctx, *current);
                    } else {
                            auto itr = com_by_root.lower_bound(current->root_comment);
                            while (itr != com_by_root.end() &&
                                   itr->root_comment == current->root_comment) {
                                    const auto &comment = *itr;
                                    ++itr;
                                    ctx.total_reward_shares2 = gpo.total_reward_shares2;
                                    ctx.total_reward_fund_steem = gpo.total_reward_fund_steem;

                                    // This extra logic is for when the funds are created in HF 16. We are using this data to preload
                                    // recent rshares 2 to prevent any downtime in payouts at HF 17. After HF 17, we can capture
                                    // the value of recent rshare 2 and set it at the hardfork instead of computing it every reindex
                                    if (funds.size() && comment.net_rshares > 0) {
                                            const auto &rf = get_reward_fund(comment);
                                            funds[rf.id._id].recent_rshares2 += utilities::calculate_vshares(comment.net_rshares.value, rf);
                                    }

                                    auto reward = cashout_comment_helper(ctx, comment);

                                    if (reward > 0) {
                                            modify(get_dynamic_global_properties(), [&](dynamic_global_property_object &p) {
                                                p.total_reward_fund_steem.amount -= reward;
                                            });
                                    }
                            }
                    }
                    current = cidx.begin();
            }

            if (funds.size()) {
                    for (size_t i = 0; i < funds.size(); i++) {
                            modify(get<reward_fund_object, by_id>(reward_fund_id_type(i)), [&](reward_fund_object &rfo) {
                                rfo.recent_rshares2 = funds[i].recent_rshares2;
                                rfo.reward_balance -= funds[i].steem_awarded;
                            });
                    }
            }
    }

    const comment_object &get_comment(const account_name_type &author, const shared_string &permlink) const {
            try {
                    return get<comment_object, by_permlink>(boost::make_tuple(author, permlink));
            } FC_CAPTURE_AND_RETHROW((author)(permlink))
    }

    const comment_object *find_comment(const account_name_type &author, const shared_string &permlink) const {
            return find<comment_object, by_permlink>(boost::make_tuple(author, permlink));
    }

    const comment_object &get_comment(const account_name_type &author, const string &permlink) const {
            try {
                    return get<comment_object, by_permlink>(boost::make_tuple(author, permlink));
            } FC_CAPTURE_AND_RETHROW((author)(permlink))
    }

    const comment_object *find_comment(const account_name_type &author, const string &permlink) const {
            return find<comment_object, by_permlink>(boost::make_tuple(author, permlink));
    }


    share_type cashout_comment_helper(utilities::comment_reward_context &ctx, const comment_object &comment) {
            try {
                    share_type claimed_reward = 0;

                    if (comment.net_rshares > 0) {
                            fill_comment_reward_context_local_state(ctx, comment);

                            const share_type reward = has_hardfork(STEEMIT_HARDFORK_0_17__86)
                                                      ? utilities::get_rshare_reward(ctx, get_reward_fund(comment))
                                                      : utilities::get_rshare_reward(ctx);
                            uint128_t reward_tokens = uint128_t(reward.value);

                            if (reward_tokens > 0) {
                                    share_type curation_tokens = ((reward_tokens *
                                                                   get_curation_rewards_percent(comment)) /
                                                                  STEEMIT_100_PERCENT).to_uint64();

                                    share_type author_tokens =
                                            reward_tokens.to_uint64() - curation_tokens;

                                    author_tokens += pay_curators(comment, curation_tokens);

                                    claimed_reward = author_tokens + curation_tokens;

                                    share_type total_beneficiary = 0;

                                    for (auto &b : comment.beneficiaries) {
                                            auto benefactor_tokens =
                                                    (author_tokens * b.weight) /
                                                    STEEMIT_100_PERCENT;
                                            auto vest_created = create_vesting(get_account(b.account),
                                                                               benefactor_tokens);
                                            references.push_virtual_operation(
                                                    comment_benefactor_reward_operation(b.account, comment.author,
                                                                                        to_string(comment.permlink),
                                                                                        vest_created));
                                            total_beneficiary += benefactor_tokens;
                                    }

                                    author_tokens -= total_beneficiary;

                                    auto sbd_steem = (author_tokens *
                                                      comment.percent_steem_dollars) /
                                                     (2 * STEEMIT_100_PERCENT);
                                    auto vesting_steem = author_tokens - sbd_steem;

                                    const auto &author = get_account(comment.author);
                                    auto vest_created = create_vesting(author, vesting_steem);
                                    auto sbd_payout = create_sbd(author, sbd_steem);

                                    references.adjust_total_payout(comment, sbd_payout.first +
                                                                            to_sbd(sbd_payout.second +
                                                                                   asset(vesting_steem, STEEM_SYMBOL)),
                                                                   to_sbd(asset(curation_tokens, STEEM_SYMBOL)),
                                                                   to_sbd(asset(total_beneficiary, STEEM_SYMBOL)));


                                    /*if( sbd_created.symbol == SBD_SYMBOL )
                                       adjust_total_payout( comment, sbd_created + to_sbd( asset( vesting_steem, STEEM_SYMBOL ) ), to_sbd( asset( reward_tokens.to_uint64() - author_tokens, STEEM_SYMBOL ) ) );
                                    else
                                       adjust_total_payout( comment, to_sbd( asset( vesting_steem + sbd_steem, STEEM_SYMBOL ) ), to_sbd( asset( reward_tokens.to_uint64() - author_tokens, STEEM_SYMBOL ) ) );
                                       */


                                    references.push_virtual_operation(
                                            author_reward_operation(comment.author, to_string(comment.permlink),
                                                                    sbd_payout.first, sbd_payout.second, vest_created));
                                    references.push_virtual_operation(
                                            comment_reward_operation(comment.author, to_string(comment.permlink),
                                                                     to_sbd(asset(claimed_reward, STEEM_SYMBOL))));

#ifndef STEEMIT_BUILD_LOW_MEMORY
                                    references.modify(comment, [&](comment_object &c) {
                                        c.author_rewards += author_tokens;
                                    });

                                    references.modify(get_account(comment.author), [&](account_object &a) {
                                        a.posting_rewards += author_tokens;
                                    });
#endif

                            }

                            if (!has_hardfork(STEEMIT_HARDFORK_0_17__86)) {
                                    references.adjust_rshares2(comment,
                                                               utilities::calculate_vshares(comment.net_rshares.value),
                                                               0);
                            }

                            references.modify(references.get_dynamic_global_properties(),
                                              [&](dynamic_global_property_object &p) {
                                                  p.total_reward_fund_steem.amount -= reward;
                                              });

                            fc::uint128_t old_rshares2 = utilities::calculate_vshares(comment.net_rshares.value);
                            references.adjust_rshares2(comment, old_rshares2, 0);
                    }

                    references.modify(comment, [&](comment_object &c) {
                        /**
                        * A payout is only made for positive rshares, negative rshares hang around
                        * for the next time this post might get an upvote.
                        */
                        if (c.net_rshares > 0) {
                                c.net_rshares = 0;
                        }
                        c.children_abs_rshares = 0;
                        c.abs_rshares = 0;
                        c.vote_rshares = 0;
                        c.total_vote_weight = 0;
                        c.max_cashout_time = fc::time_point_sec::maximum();

                        if (has_hardfork(STEEMIT_HARDFORK_0_17__91)) {
                                c.cashout_time = fc::time_point_sec::maximum();
                        } else if (c.parent_author == STEEMIT_ROOT_POST_PARENT) {
                                if (has_hardfork(STEEMIT_HARDFORK_0_12__177) &&
                                    c.last_payout == fc::time_point_sec::min()) {
                                        c.cashout_time = head_block_time() +
                                                         STEEMIT_SECOND_CASHOUT_WINDOW;
                                } else {
                                        c.cashout_time = fc::time_point_sec::maximum();
                                }
                        }

                        c.last_payout = head_block_time();
                    });

                    references.push_virtual_operation(
                            comment_payout_update_operation(comment.author, to_string(comment.permlink)));

                    const auto &vote_idx = get_index<comment_vote_index>().indices().get<by_comment_voter>();
                    auto vote_itr = vote_idx.lower_bound(comment.id);
                    while (vote_itr != vote_idx.end() &&
                           vote_itr->comment == comment.id) {
                            const auto &cur_vote = *vote_itr;
                            ++vote_itr;
                            if (!has_hardfork(STEEMIT_HARDFORK_0_12__177) ||
                                calculate_discussion_payout_time(comment) !=
                                fc::time_point_sec::maximum()) {
                                    references.modify(cur_vote, [&](comment_vote_object &cvo) {
                                        cvo.num_changes = -1;
                                    });
                            } else {
#ifdef CLEAR_VOTES
                                    remove(cur_vote);
#endif
                            }
                    }
                    return claimed_reward;
            }
            FC_CAPTURE_AND_RETHROW((comment))
    }


    time_point_sec get_payout_extension_time(const comment_object &input_comment, const asset &input_cost) const {
        FC_ASSERT(input_cost.symbol ==
                  SBD_SYMBOL, "Extension payment should be in SBD");
        FC_ASSERT(
                input_cost.amount / STEEMIT_PAYOUT_EXTENSION_COST_PER_DAY >
                0, "Extension payment should cover more than a day");
        return fc::time_point::now() +
               fc::seconds(((input_cost.amount.value * 60 * 60 * 24 *
                             input_comment.net_rshares.value) /
                            STEEMIT_PAYOUT_EXTENSION_COST_PER_DAY));
    }


    uint16_t get_curation_rewards_percent(const comment_object &c) const {
        if (has_hardfork(STEEMIT_HARDFORK_0_17__86) &&
            c.parent_author != STEEMIT_ROOT_POST_PARENT) {
            return 0;
        } else if (has_hardfork(STEEMIT_HARDFORK_0_8__116)) {
            return STEEMIT_1_PERCENT * 25;
        } else {
            return STEEMIT_1_PERCENT * 50;
        }
    }

protected:
    database_basic &references;

};
}}

#endif