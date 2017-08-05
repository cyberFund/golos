#include <steemit/chain/evaluators/vote_evaluator.hpp>
void steemit::chain::vote_evaluator::do_apply(const protocol::vote_operation &o) {
    try {


        const auto &comment = this->_db.get_comment(o.author, o.permlink);
        const auto &voter = this->_db.get_account(o.voter);

        if (this->_db.has_hardfork(STEEMIT_HARDFORK_0_10)) {
            FC_ASSERT(!(voter.owner_challenged ||
                        voter.active_challenged),
                      "Operation cannot be processed because the account is currently challenged.");
        }

        FC_ASSERT(voter.can_vote, "Voter has declined their voting rights.");

        if (o.weight > 0) {
            FC_ASSERT(comment.allow_votes, "Votes are not allowed on the comment.");
        }

        if (this->_db.has_hardfork(STEEMIT_HARDFORK_0_12__177) &&
            this->_db.calculate_discussion_payout_time(comment) ==
            fc::time_point_sec::maximum()) {
#ifndef CLEAR_VOTES
            const auto &comment_vote_idx = this->_db.get_index<comment_vote_index>().indices().get<by_comment_voter>();
            auto itr = comment_vote_idx.find(std::make_tuple(comment.id, voter.id));

            if (itr == comment_vote_idx.end())
                this->_db.create<comment_vote_object>([&](comment_vote_object &cvo) {
                    cvo.voter = voter.id;
                    cvo.comment = comment.id;
                    cvo.vote_percent = o.weight;
                    cvo.last_update = this->_db.head_block_time();
                });
            else
                this->_db.modify(*itr, [&](comment_vote_object &cvo) {
                    cvo.vote_percent = o.weight;
                    cvo.last_update = this->_db.head_block_time();
                });
#endif
            return;
        }

        const auto &comment_vote_idx = this->_db.get_index<comment_vote_index>().indices().get<by_comment_voter>();
        auto itr = comment_vote_idx.find(std::make_tuple(comment.id, voter.id));

        int64_t elapsed_seconds = (this->_db.head_block_time() -
                                   voter.last_vote_time).to_seconds();

        if (this->_db.has_hardfork(STEEMIT_HARDFORK_0_11)) {
            FC_ASSERT(elapsed_seconds >=
                      STEEMIT_MIN_VOTE_INTERVAL_SEC, "Can only vote once every 3 seconds.");
        }

        int64_t regenerated_power =
                (STEEMIT_100_PERCENT * elapsed_seconds) /
                STEEMIT_VOTE_REGENERATION_SECONDS;
        int64_t current_power = std::min(int64_t(voter.voting_power +
                                                 regenerated_power), int64_t(STEEMIT_100_PERCENT));
        FC_ASSERT(current_power >
                  0, "Account currently does not have voting power.");

        int64_t abs_weight = abs(o.weight);
        int64_t used_power =
                (current_power * abs_weight) / STEEMIT_100_PERCENT;

        const dynamic_global_property_object &dgpo = this->_db.get_dynamic_global_properties();

        // used_power = (current_power * abs_weight / STEEMIT_100_PERCENT) * (reserve / max_vote_denom)
        // The second multiplication is rounded up as of HF 259
        int64_t max_vote_denom = dgpo.vote_regeneration_per_day *
                                 STEEMIT_VOTE_REGENERATION_SECONDS /
                                 (60 * 60 * 24);
        FC_ASSERT(max_vote_denom > 0);

        if (!this->_db.has_hardfork(STEEMIT_HARDFORK_0_14__259)) {
            used_power = (used_power / max_vote_denom) + 1;
        } else {
            used_power =
                    (used_power + max_vote_denom - 1) / max_vote_denom;
        }
        FC_ASSERT(used_power <=
                  current_power, "Account does not have enough power to vote.");

        int64_t abs_rshares = (
                (uint128_t(voter.effective_vesting_shares().amount.value) *
                 used_power) / (STEEMIT_100_PERCENT)).to_uint64();
        if (!this->_db.has_hardfork(STEEMIT_HARDFORK_0_14__259) &&
            abs_rshares == 0) {
            abs_rshares = 1;
        }

        if (this->_db.has_hardfork(STEEMIT_HARDFORK_0_14__259)) {
            FC_ASSERT(abs_rshares > STEEMIT_VOTE_DUST_THRESHOLD ||
                      o.weight ==
                      0, "Voting weight is too small, please accumulate more voting power or steem power.");
        } else if (this->_db.has_hardfork(STEEMIT_HARDFORK_0_13__248)) {
            FC_ASSERT(abs_rshares > STEEMIT_VOTE_DUST_THRESHOLD ||
                      abs_rshares ==
                      1, "Voting weight is too small, please accumulate more voting power or steem power.");
        }

        // Lazily delete vote
        if (itr != comment_vote_idx.end() && itr->num_changes == -1) {
            if (this->_db.is_producing() ||
                this->_db.has_hardfork(STEEMIT_HARDFORK_0_12__177)) {
                FC_ASSERT(false, "Cannot vote again on a comment after payout.");
            }

            this->_db.remove(*itr);
            itr = comment_vote_idx.end();
        }

        if (itr == comment_vote_idx.end()) {
            FC_ASSERT(o.weight != 0, "Vote weight cannot be 0.");
            /// this is the rshares voting for or against the post
            int64_t rshares = o.weight < 0 ? -abs_rshares : abs_rshares;

            if (rshares > 0 &&
                this->_db.has_hardfork(STEEMIT_HARDFORK_0_7)) {
                if (this->_db.has_hardfork(STEEMIT_HARDFORK_0_17__91)) {
                    FC_ASSERT(this->_db.head_block_time() <
                              comment.cashout_time -
                              STEEMIT_UPVOTE_LOCKOUT,
                              "Cannot increase reward of post within the last minute before payout.");
                } else {
                    FC_ASSERT(this->_db.head_block_time() <
                              this->_db.calculate_discussion_payout_time(comment) -
                              STEEMIT_UPVOTE_LOCKOUT,
                              "Cannot increase reward of post within the last minute before payout.");
                }

            }

            //used_power /= (50*7); /// a 100% vote means use .28% of voting power which should force users to spread their votes around over 50+ posts day for a week
            //if( used_power == 0 ) used_power = 1;

            this->_db.modify(voter, [&](account_object &a) {
                a.voting_power = current_power - used_power;
                a.last_vote_time = this->_db.head_block_time();
            });

            /// if the current net_rshares is less than 0, the post is getting 0 rewards so it is not factored into total rshares^2
            fc::uint128_t old_rshares = std::max(comment.net_rshares.value, int64_t(0));
            const auto &root = this->_db.get(comment.root_comment);
            auto old_root_abs_rshares = root.children_abs_rshares.value;

            fc::uint128_t avg_cashout_sec;

            if (!this->_db.has_hardfork(STEEMIT_HARDFORK_0_17__91)) {
                fc::uint128_t cur_cashout_time_sec = this->_db.calculate_discussion_payout_time(
                        comment).sec_since_epoch();
                fc::uint128_t new_cashout_time_sec;

                if (this->_db.has_hardfork(STEEMIT_HARDFORK_0_12__177) &&
                    !this->_db.has_hardfork(STEEMIT_HARDFORK_0_13__257)) {
                    new_cashout_time_sec =
                            this->_db.head_block_time().sec_since_epoch() +
                            STEEMIT_CASHOUT_WINDOW_SECONDS_PRE_HF17;
                } else {
                    new_cashout_time_sec =
                            this->_db.head_block_time().sec_since_epoch() +
                            STEEMIT_CASHOUT_WINDOW_SECONDS_PRE_HF12;
                }

                avg_cashout_sec =
                        (cur_cashout_time_sec * old_root_abs_rshares +
                         new_cashout_time_sec * abs_rshares) /
                        (old_root_abs_rshares + abs_rshares);
            }

            FC_ASSERT(abs_rshares > 0, "Cannot vote with 0 rshares.");

            auto old_vote_rshares = comment.vote_rshares;

            this->_db.modify(comment, [&](comment_object &c) {
                c.net_rshares += rshares;
                c.abs_rshares += abs_rshares;
                if (rshares > 0) {
                    c.vote_rshares += rshares;
                }
                if (rshares > 0) {
                    c.net_votes++;
                } else {
                    c.net_votes--;
                }
                if (!this->_db.has_hardfork(STEEMIT_HARDFORK_0_6__114) &&
                    c.net_rshares == -c.abs_rshares) {
                    FC_ASSERT(c.net_votes <
                              0, "Comment has negative net votes?");
                }
            });

            this->_db.modify(root, [&](comment_object &c) {
                c.children_abs_rshares += abs_rshares;
                if (!this->_db.has_hardfork(STEEMIT_HARDFORK_0_17__91)) {
                    if (this->_db.has_hardfork(STEEMIT_HARDFORK_0_12__177) &&
                        c.last_payout > fc::time_point_sec::min()) {
                        c.cashout_time = c.last_payout +
                                         STEEMIT_SECOND_CASHOUT_WINDOW;
                    } else {
                        c.cashout_time = fc::time_point_sec(std::min(uint32_t(avg_cashout_sec.to_uint64()),
                                                                     c.max_cashout_time.sec_since_epoch()));
                    }

                    if (c.max_cashout_time ==
                        fc::time_point_sec::maximum()) {
                        c.max_cashout_time =
                                this->_db.head_block_time() +
                                fc::seconds(STEEMIT_MAX_CASHOUT_WINDOW_SECONDS);
                    }
                }
            });

            fc::uint128_t new_rshares = std::max(comment.net_rshares.value, int64_t(0));

            /// calculate rshares2 value
            new_rshares = utilities::calculate_vshares(new_rshares);
            old_rshares = utilities::calculate_vshares(old_rshares);

            uint64_t max_vote_weight = 0;

            /** this verifies uniqueness of voter
*
*  cv.weight / c.total_vote_weight ==> % of rshares increase that is accounted for by the vote
*
*  W(R) = B * R / ( R + 2S )
*  W(R) is bounded above by B. B is fixed at 2^64 - 1, so all weights fit in a 64 bit integer.
*
*  The equation for an individual vote is:
*    W(R_N) - W(R_N-1), which is the delta increase of proportional weight
*
*  c.total_vote_weight =
*    W(R_1) - W(R_0) +
*    W(R_2) - W(R_1) + ...
*    W(R_N) - W(R_N-1) = W(R_N) - W(R_0)
*
*  Since W(R_0) = 0, c.total_vote_weight is also bounded above by B and will always fit in a 64 bit integer.
*
**/
            this->_db.create<comment_vote_object>([&](comment_vote_object &cv) {
                cv.voter = voter.id;
                cv.comment = comment.id;
                cv.rshares = rshares;
                cv.vote_percent = o.weight;
                cv.last_update = this->_db.head_block_time();

                bool curation_reward_eligible = rshares > 0 &&
                                                (comment.last_payout ==
                                                 fc::time_point_sec()) &&
                                                comment.allow_curation_rewards;

                if (curation_reward_eligible &&
                    this->_db.has_hardfork(STEEMIT_HARDFORK_0_17__86)) {
                    curation_reward_eligible =
                            this->_db.get_curation_rewards_percent(comment) >
                            0;
                }

                if (curation_reward_eligible) {
                    if (comment.created <
                        fc::time_point_sec(STEEMIT_HARDFORK_0_6_REVERSE_AUCTION_TIME)) {
                        u512 rshares3(rshares);
                        u256 total2(comment.abs_rshares.value);

                        if (!this->_db.has_hardfork(STEEMIT_HARDFORK_0_1)) {
                            rshares3 *= 10000;
                            total2 *= 10000;
                        }

                        rshares3 = rshares3 * rshares3 * rshares3;

                        total2 *= total2;
                        cv.weight = static_cast<uint64_t>( rshares3 /
                                                           total2 );
                    } else {// cv.weight = W(R_1) - W(R_0)
                        const uint128_t two_s =
                                2 * utilities::get_content_constant_s();
                        if (this->_db.has_hardfork(STEEMIT_HARDFORK_0_1)) {
                            uint64_t old_weight = (
                                    (std::numeric_limits<uint64_t>::max() *
                                     fc::uint128_t(old_vote_rshares.value)) /
                                    (2 * two_s +
                                     old_vote_rshares.value)).to_uint64();
                            uint64_t new_weight = (
                                    (std::numeric_limits<uint64_t>::max() *
                                     fc::uint128_t(comment.vote_rshares.value)) /
                                    (2 * two_s +
                                     comment.vote_rshares.value)).to_uint64();
                            cv.weight = new_weight - old_weight;
                        } else {
                            uint64_t old_weight = (
                                    (std::numeric_limits<uint64_t>::max() *
                                     fc::uint128_t(10000 *
                                                   old_vote_rshares.value)) /
                                    (2 * two_s +
                                     (10000 *
                                      old_vote_rshares.value))).to_uint64();
                            uint64_t new_weight = (
                                    (std::numeric_limits<uint64_t>::max() *
                                     fc::uint128_t(10000 *
                                                   comment.vote_rshares.value)) /
                                    (2 * two_s +
                                     (10000 *
                                      comment.vote_rshares.value))).to_uint64();
                            cv.weight = new_weight - old_weight;
                        }
                    }

                    max_vote_weight = cv.weight;

                    if (this->_db.head_block_time() >
                        fc::time_point_sec(
                                STEEMIT_HARDFORK_0_6_REVERSE_AUCTION_TIME))  /// start enforcing this prior to the hardfork
                    {
                        /// discount weight by time
                        uint128_t w(max_vote_weight);
                        uint64_t delta_t = std::min(uint64_t((
                                                                     cv.last_update -
                                                                     comment.created).to_seconds()),
                                                    uint64_t(STEEMIT_REVERSE_AUCTION_WINDOW_SECONDS));

                        w *= delta_t;
                        w /= STEEMIT_REVERSE_AUCTION_WINDOW_SECONDS;
                        cv.weight = w.to_uint64();
                    }
                } else {
                    cv.weight = 0;
                }
            });

            if (max_vote_weight) // Optimization
            {
                this->_db.modify(comment, [&](comment_object &c) {
                    c.total_vote_weight += max_vote_weight;
                });
            }

            if (!this->_db.has_hardfork(STEEMIT_HARDFORK_0_17__86)) {
                this->_db.adjust_rshares2(comment, old_rshares, new_rshares);
            }
        } else {
            FC_ASSERT(itr->num_changes <
                      STEEMIT_MAX_VOTE_CHANGES,
                      "Voter has used the maximum number of vote changes on this comment.");

            if (this->_db.is_producing() ||
                this->_db.has_hardfork(STEEMIT_HARDFORK_0_6__112)) {
                FC_ASSERT(itr->vote_percent !=
                          o.weight, "You have already voted in a similar way.");
            }

            /// this is the rshares voting for or against the post
            int64_t rshares = o.weight < 0 ? -abs_rshares : abs_rshares;

            if (itr->rshares < rshares &&
                this->_db.has_hardfork(STEEMIT_HARDFORK_0_7)) {
                FC_ASSERT(this->_db.head_block_time() <
                          this->_db.calculate_discussion_payout_time(comment) -
                          STEEMIT_UPVOTE_LOCKOUT,
                          "Cannot increase payout within last minute before payout.");
            }

            this->_db.modify(voter, [&](account_object &a) {
                a.voting_power = current_power - used_power;
                a.last_vote_time = this->_db.head_block_time();
            });

            /// if the current net_rshares is less than 0, the post is getting 0 rewards so it is not factored into total rshares^2
            fc::uint128_t old_rshares = std::max(comment.net_rshares.value, int64_t(0));
            const auto &root = this->_db.get(comment.root_comment);
            auto old_root_abs_rshares = root.children_abs_rshares.value;

            fc::uint128_t avg_cashout_sec;

            if (!this->_db.has_hardfork(STEEMIT_HARDFORK_0_17__91)) {
                fc::uint128_t cur_cashout_time_sec = this->_db.calculate_discussion_payout_time(
                        comment).sec_since_epoch();
                fc::uint128_t new_cashout_time_sec;

                if (this->_db.has_hardfork(STEEMIT_HARDFORK_0_12__177) &&
                    !this->_db.has_hardfork(STEEMIT_HARDFORK_0_13__257)) {
                    new_cashout_time_sec =
                            this->_db.head_block_time().sec_since_epoch() +
                            STEEMIT_CASHOUT_WINDOW_SECONDS_PRE_HF17;
                } else {
                    new_cashout_time_sec =
                            this->_db.head_block_time().sec_since_epoch() +
                            STEEMIT_CASHOUT_WINDOW_SECONDS_PRE_HF12;
                }

                if (this->_db.has_hardfork(STEEMIT_HARDFORK_0_14__259) &&
                    abs_rshares == 0) {
                    avg_cashout_sec = cur_cashout_time_sec;
                } else {
                    avg_cashout_sec = (cur_cashout_time_sec *
                                       old_root_abs_rshares +
                                       new_cashout_time_sec *
                                       abs_rshares) /
                                      (old_root_abs_rshares +
                                       abs_rshares);
                }
            }

            this->_db.modify(comment, [&](comment_object &c) {
                c.net_rshares -= itr->rshares;
                c.net_rshares += rshares;
                c.abs_rshares += abs_rshares;

                /// TODO: figure out how to handle remove a vote (rshares == 0 )
                if (rshares > 0 && itr->rshares < 0) {
                    c.net_votes += 2;
                } else if (rshares > 0 && itr->rshares == 0) {
                    c.net_votes += 1;
                } else if (rshares == 0 && itr->rshares < 0) {
                    c.net_votes += 1;
                } else if (rshares == 0 && itr->rshares > 0) {
                    c.net_votes -= 1;
                } else if (rshares < 0 && itr->rshares == 0) {
                    c.net_votes -= 1;
                } else if (rshares < 0 && itr->rshares > 0) {
                    c.net_votes -= 2;
                }
            });

            this->_db.modify(root, [&](comment_object &c) {
                c.children_abs_rshares += abs_rshares;
                if (!this->_db.has_hardfork(STEEMIT_HARDFORK_0_17__91)) {
                    if (this->_db.has_hardfork(STEEMIT_HARDFORK_0_12__177) &&
                        c.last_payout > fc::time_point_sec::min()) {
                        c.cashout_time = c.last_payout +
                                         STEEMIT_SECOND_CASHOUT_WINDOW;
                    } else {
                        c.cashout_time = fc::time_point_sec(std::min(uint32_t(avg_cashout_sec.to_uint64()),
                                                                     c.max_cashout_time.sec_since_epoch()));
                    }

                    if (c.max_cashout_time ==
                        fc::time_point_sec::maximum()) {
                        c.max_cashout_time =
                                this->_db.head_block_time() +
                                fc::seconds(STEEMIT_MAX_CASHOUT_WINDOW_SECONDS);
                    }
                }
            });

            fc::uint128_t new_rshares = std::max(comment.net_rshares.value, int64_t(0));

            /// calculate rshares2 value
            new_rshares = utilities::calculate_vshares(new_rshares);
            old_rshares = utilities::calculate_vshares(old_rshares);

            this->_db.modify(comment, [&](comment_object &c) {
                c.total_vote_weight -= itr->weight;
            });

            this->_db.modify(*itr, [&](comment_vote_object &cv) {
                cv.rshares = rshares;
                cv.vote_percent = o.weight;
                cv.last_update = this->_db.head_block_time();
                cv.weight = 0;
                cv.num_changes += 1;
            });

            if (!this->_db.has_hardfork(STEEMIT_HARDFORK_0_17__86)) {
                this->_db.adjust_rshares2(comment, old_rshares, new_rshares);
            }

        }

    }
    FC_CAPTURE_AND_RETHROW((o))
}
