#ifndef GOLOS_HARDFORK_PROPERTY_POLICY_HPP
#define GOLOS_HARDFORK_PROPERTY_POLICY_HPP

#include "generic_policy.hpp"

namespace steemit {
namespace chain {
struct hardfork_property_policy: public generic_policy {

    hardfork_property_policy() = default;

    hardfork_property_policy(const hardfork_property_policy &) = default;

    hardfork_property_policy &operator=(const hardfork_property_policy &) = default;

    hardfork_property_policy(hardfork_property_policy &&) = default;

    hardfork_property_policy &operator=(hardfork_property_policy &&) = default;

    virtual ~hardfork_property_policy() = default;

    hardfork_property_policy(database_basic &ref, evaluator_registry <operation> &evaluator_registry_) : generic_policy(ref) {
    }

    const hardfork_property_object &get_hardfork_property_object() const {
        try {
            return references.get<hardfork_property_object>();
        } FC_CAPTURE_AND_RETHROW()
    }

    void process_hardforks() {
        try {
            // If there are upcoming hardforks and the next one is later, do nothing
            const auto &hardforks = get_hardfork_property_object();

            if (has_hardfork(STEEMIT_HARDFORK_0_5__54)) {
                while (
                        _hardfork_versions[hardforks.last_hardfork] < hardforks.next_hardfork
                       &&
                       hardforks.next_hardfork_time <= head_block_time()
                ) {
                    if (hardforks.last_hardfork < STEEMIT_NUM_HARDFORKS) {
                        apply_hardfork(hardforks.last_hardfork + 1);
                    } else {
                        throw unknown_hardfork_exception();
                    }
                }
            } else {
                while (hardforks.last_hardfork < STEEMIT_NUM_HARDFORKS
                       && _hardfork_times[hardforks.last_hardfork + 1] <=
                          head_block_time()
                       &&
                       hardforks.last_hardfork < STEEMIT_HARDFORK_0_5__54) {
                    apply_hardfork(hardforks.last_hardfork + 1);
                }
            }
        }
        FC_CAPTURE_AND_RETHROW()
    }

    bool has_hardfork(uint32_t hardfork) const {
        return get_hardfork_property_object().processed_hardforks.size() >
               hardfork;
    }

    void set_hardfork(uint32_t hardfork, bool apply_now) {
        auto const &hardforks = get_hardfork_property_object();

        for (uint32_t i = hardforks.last_hardfork + 1;
             i <= hardfork && i <= STEEMIT_NUM_HARDFORKS; i++) {
            if (i <= STEEMIT_HARDFORK_0_5__54) {
                _hardfork_times[i] = head_block_time();
            } else {
                references.modify(hardforks, [&](hardfork_property_object &hpo) {
                    hpo.next_hardfork = _hardfork_versions[i];
                    hpo.next_hardfork_time = head_block_time();
                });
            }

            if (apply_now) {
                apply_hardfork(i);
            }
        }
    }

    void apply_hardfork(uint32_t hardfork) {
        if (_log_hardforks) {
            elog("HARDFORK ${hf} at block ${b}", ("hf", hardfork)("b", head_block_num()));
        }

        switch (hardfork) {
            case STEEMIT_HARDFORK_0_1:
                perform_vesting_share_split(10000);
#ifdef STEEMIT_BUILD_TESTNET
            {
                    custom_operation test_op;
                    string op_msg = "Testnet: Hardfork applied";
                    test_op.data = vector<char>(op_msg.begin(), op_msg.end());
                    test_op.required_auths.insert(STEEMIT_INIT_MINER_NAME);
                    operation op = test_op;   // we need the operation object to live to the end of this scope
                    operation_notification note(op);
                    notify_pre_apply_operation(note);
                    notify_post_apply_operation(note);
                }
                break;
#endif
                break;
            case STEEMIT_HARDFORK_0_2:
                retally_witness_votes();
                break;
            case STEEMIT_HARDFORK_0_3:
                retally_witness_votes();
                break;
            case STEEMIT_HARDFORK_0_4:
                reset_virtual_schedule_time();
                break;
            case STEEMIT_HARDFORK_0_5:
                break;
            case STEEMIT_HARDFORK_0_6:
                retally_witness_vote_counts();
                retally_comment_children();
                break;
            case STEEMIT_HARDFORK_0_7:
                break;
            case STEEMIT_HARDFORK_0_8:
                retally_witness_vote_counts(true);
                break;
            case STEEMIT_HARDFORK_0_9: {

            }
                break;
            case STEEMIT_HARDFORK_0_10:
                retally_liquidity_weight();
                break;
            case STEEMIT_HARDFORK_0_11:
                break;
            case STEEMIT_HARDFORK_0_12: {
                const auto &comment_idx = get_index<comment_index>().indices();

                for (auto itr = comment_idx.begin();
                     itr != comment_idx.end(); ++itr) {
                    // At the hardfork time, all new posts with no votes get their cashout time set to +12 hrs from head block time.
                    // All posts with a payout get their cashout time set to +30 days. This hardfork takes place within 30 days
                    // initial payout so we don't have to handle the case of posts that should be frozen that aren't
                    if (itr->parent_author == STEEMIT_ROOT_POST_PARENT) {
                        // Post has not been paid out and has no votes (cashout_time == 0 === net_rshares == 0, under current semmantics)
                        if (itr->last_payout == fc::time_point_sec::min() &&
                            itr->cashout_time ==
                            fc::time_point_sec::maximum()) {
                            modify(*itr, [&](comment_object &c) {
                                c.cashout_time = head_block_time() +
                                                 STEEMIT_CASHOUT_WINDOW_SECONDS_PRE_HF17;
                            });
                        }
                            // Has been paid out, needs to be on second cashout window
                        else if (itr->last_payout > fc::time_point_sec()) {
                            modify(*itr, [&](comment_object &c) {
                                c.cashout_time = c.last_payout +
                                                 STEEMIT_SECOND_CASHOUT_WINDOW;
                            });
                        }
                    }
                }

                references.modify(get<account_authority_object, by_account>(STEEMIT_MINER_ACCOUNT), [&](account_authority_object &auth) {
                    auth.posting = authority();
                    auth.posting.weight_threshold = 1;
                });

                references.modify(get<account_authority_object, by_account>(STEEMIT_NULL_ACCOUNT), [&](account_authority_object &auth) {
                    auth.posting = authority();
                    auth.posting.weight_threshold = 1;
                });

                references.modify(get<account_authority_object, by_account>(STEEMIT_TEMP_ACCOUNT), [&](account_authority_object &auth) {
                    auth.posting = authority();
                    auth.posting.weight_threshold = 1;
                });
            }
                break;
            case STEEMIT_HARDFORK_0_13:
                break;
            case STEEMIT_HARDFORK_0_14:
                break;
            case STEEMIT_HARDFORK_0_15:
                break;
            case STEEMIT_HARDFORK_0_16: {
                modify(get_feed_history(), [&](feed_history_object &fho) {
                    while (fho.price_history.size() >
                           STEEMIT_FEED_HISTORY_WINDOW) {
                        fho.price_history.pop_front();
                    }
                });

                for (const std::string &acc : hardfork16::get_compromised_accounts()) {
                    const account_object *account = find_account(acc);
                    if (account == nullptr) {
                        continue;
                    }

                    update_owner_authority(*account, authority(1, public_key_type("GLS8hLtc7rC59Ed7uNVVTXtF578pJKQwMfdTvuzYLwUi8GkNTh5F6"), 1));

                    references.modify(get<account_authority_object, by_account>(account->name), [&](account_authority_object &auth) {
                        auth.active = authority(1, public_key_type("GLS8hLtc7rC59Ed7uNVVTXtF578pJKQwMfdTvuzYLwUi8GkNTh5F6"), 1);
                        auth.posting = authority(1, public_key_type("GLS8hLtc7rC59Ed7uNVVTXtF578pJKQwMfdTvuzYLwUi8GkNTh5F6"), 1);
                    });
                }

                create<reward_fund_object>([&](reward_fund_object &rfo) {
                    rfo.name = STEEMIT_POST_REWARD_FUND_NAME;
                    rfo.last_update = head_block_time();
                    rfo.percent_content_rewards = 0;
                    rfo.content_constant = utilities::get_content_constant_s().to_uint64();
                });

                create<reward_fund_object>([&](reward_fund_object &rfo) {
                    rfo.name = STEEMIT_COMMENT_REWARD_FUND_NAME;
                    rfo.last_update = head_block_time();
                    rfo.percent_content_rewards = 0;
                    rfo.content_constant = utilities::get_content_constant_s().to_uint64();
                });
            }
                break;

            case STEEMIT_HARDFORK_0_17: {
                const auto &gpo = get_dynamic_global_properties();
                auto reward_steem = gpo.total_reward_fund_steem;


                references.modify(get<reward_fund_object, by_name>(STEEMIT_POST_REWARD_FUND_NAME), [&](reward_fund_object &rfo) {
                    rfo.percent_content_rewards = STEEMIT_POST_REWARD_FUND_PERCENT;
                    rfo.reward_balance = asset((reward_steem.amount.value *
                                                rfo.percent_content_rewards) /
                                               STEEMIT_100_PERCENT, STEEM_SYMBOL);
                    reward_steem -= rfo.reward_balance;

                });

                references.modify(get<reward_fund_object, by_name>(STEEMIT_COMMENT_REWARD_FUND_NAME), [&](reward_fund_object &rfo) {
                    rfo.percent_content_rewards = STEEMIT_COMMENT_REWARD_FUND_PERCENT;
                    rfo.reward_balance = reward_steem;
                });

                references.modify(gpo, [&](dynamic_global_property_object &g) {
                    g.total_reward_fund_steem = asset(0, STEEM_SYMBOL);
                    g.total_reward_shares2 = 0;

                });

                /*
                 * For all current comments we will either keep their current cashout time, or extend it to 1 week
                 * after creation.
                 *
                 * We cannot do a simple iteration by cashout time because we are editting cashout time.
                 * More specifically, we will be adding an explicit cashout time to all comments with parents.
                 * To find all discussions that have not been paid out we fir iterate over posts by cashout time.
                 * Before the hardfork these are all root posts. Iterate over all of their children, adding each
                 * to a specific list. Next, update payout times for all discussions on the root post. This defines
                 * the min cashout time for each child in the discussion. Then iterate over the children and set
                 * their cashout time in a similar way, grabbing the root post as their inherent cashout time.
                 */
                const auto &comment_idx = get_index<comment_index, by_cashout_time>();
                const auto &by_root_idx = get_index<comment_index, by_root>();
                vector<const comment_object *> root_posts;
                root_posts.reserve(60000);
                vector<const comment_object *> replies;
                replies.reserve(100000);

                for (auto itr = comment_idx.begin();
                     itr != comment_idx.end() && itr->cashout_time <
                                                 fc::time_point_sec::maximum(); ++itr) {
                    root_posts.push_back(&(*itr));

                    for (auto reply_itr = by_root_idx.lower_bound(itr->id);
                         reply_itr != by_root_idx.end() &&
                         reply_itr->root_comment == itr->id; ++reply_itr) {
                        replies.push_back(&(*reply_itr));
                    }
                }

                for (auto itr : root_posts) {
                    references.modify(*itr, [&](comment_object &c) {
                        c.cashout_time = std::max(c.created +
                                                  STEEMIT_CASHOUT_WINDOW_SECONDS, c.cashout_time);
                        c.children_rshares2 = 0;
                    });
                }

                for (auto itr : replies) {
                    references.modify(*itr, [&](comment_object &c) {
                        c.cashout_time = std::max(calculate_discussion_payout_time(c),
                                                  c.created + STEEMIT_CASHOUT_WINDOW_SECONDS);
                        c.children_rshares2 = 0;
                    });
                }
            }
                break;

            default:
                break;
        }

        references.modify(get_hardfork_property_object(), [&](hardfork_property_object &hfp) {
            FC_ASSERT(hardfork == hfp.last_hardfork +
                                  1, "Hardfork being applied out of order", ("hardfork", hardfork)("hfp.last_hardfork", hfp.last_hardfork));
            FC_ASSERT(hfp.processed_hardforks.size() ==
                      hardfork, "Hardfork being applied out of order");
            hfp.processed_hardforks.push_back(_hardfork_times[hardfork]);
            hfp.last_hardfork = hardfork;
            hfp.current_hardfork_version = _hardfork_versions[hardfork];
            FC_ASSERT(hfp.processed_hardforks[hfp.last_hardfork] ==
                      _hardfork_times[hfp.last_hardfork], "Hardfork processing failed sanity check...");
        });

        references.push_virtual_operation(hardfork_operation(hardfork), true);
    }

    void init_hardforks() {
        _hardfork_times[0] = fc::time_point_sec(STEEMIT_GENESIS_TIME);
        _hardfork_versions[0] = hardfork_version(0, 0);
        FC_ASSERT(STEEMIT_HARDFORK_0_1 ==
                  1, "Invalid hardfork configuration");
        _hardfork_times[STEEMIT_HARDFORK_0_1] = fc::time_point_sec(STEEMIT_HARDFORK_0_1_TIME);
        _hardfork_versions[STEEMIT_HARDFORK_0_1] = STEEMIT_HARDFORK_0_1_VERSION;
        FC_ASSERT(STEEMIT_HARDFORK_0_2 ==
                  2, "Invlaid hardfork configuration");
        _hardfork_times[STEEMIT_HARDFORK_0_2] = fc::time_point_sec(STEEMIT_HARDFORK_0_2_TIME);
        _hardfork_versions[STEEMIT_HARDFORK_0_2] = STEEMIT_HARDFORK_0_2_VERSION;
        FC_ASSERT(STEEMIT_HARDFORK_0_3 ==
                  3, "Invalid hardfork configuration");
        _hardfork_times[STEEMIT_HARDFORK_0_3] = fc::time_point_sec(STEEMIT_HARDFORK_0_3_TIME);
        _hardfork_versions[STEEMIT_HARDFORK_0_3] = STEEMIT_HARDFORK_0_3_VERSION;
        FC_ASSERT(STEEMIT_HARDFORK_0_4 ==
                  4, "Invalid hardfork configuration");
        _hardfork_times[STEEMIT_HARDFORK_0_4] = fc::time_point_sec(STEEMIT_HARDFORK_0_4_TIME);
        _hardfork_versions[STEEMIT_HARDFORK_0_4] = STEEMIT_HARDFORK_0_4_VERSION;
        FC_ASSERT(STEEMIT_HARDFORK_0_5 ==
                  5, "Invalid hardfork configuration");
        _hardfork_times[STEEMIT_HARDFORK_0_5] = fc::time_point_sec(STEEMIT_HARDFORK_0_5_TIME);
        _hardfork_versions[STEEMIT_HARDFORK_0_5] = STEEMIT_HARDFORK_0_5_VERSION;
        FC_ASSERT(STEEMIT_HARDFORK_0_6 ==
                  6, "Invalid hardfork configuration");
        _hardfork_times[STEEMIT_HARDFORK_0_6] = fc::time_point_sec(STEEMIT_HARDFORK_0_6_TIME);
        _hardfork_versions[STEEMIT_HARDFORK_0_6] = STEEMIT_HARDFORK_0_6_VERSION;
        FC_ASSERT(STEEMIT_HARDFORK_0_7 ==
                  7, "Invalid hardfork configuration");
        _hardfork_times[STEEMIT_HARDFORK_0_7] = fc::time_point_sec(STEEMIT_HARDFORK_0_7_TIME);
        _hardfork_versions[STEEMIT_HARDFORK_0_7] = STEEMIT_HARDFORK_0_7_VERSION;
        FC_ASSERT(STEEMIT_HARDFORK_0_8 ==
                  8, "Invalid hardfork configuration");
        _hardfork_times[STEEMIT_HARDFORK_0_8] = fc::time_point_sec(STEEMIT_HARDFORK_0_8_TIME);
        _hardfork_versions[STEEMIT_HARDFORK_0_8] = STEEMIT_HARDFORK_0_8_VERSION;
        FC_ASSERT(STEEMIT_HARDFORK_0_9 ==
                  9, "Invalid hardfork configuration");
        _hardfork_times[STEEMIT_HARDFORK_0_9] = fc::time_point_sec(STEEMIT_HARDFORK_0_9_TIME);
        _hardfork_versions[STEEMIT_HARDFORK_0_9] = STEEMIT_HARDFORK_0_9_VERSION;
        FC_ASSERT(STEEMIT_HARDFORK_0_10 ==
                  10, "Invalid hardfork configuration");
        _hardfork_times[STEEMIT_HARDFORK_0_10] = fc::time_point_sec(STEEMIT_HARDFORK_0_10_TIME);
        _hardfork_versions[STEEMIT_HARDFORK_0_10] = STEEMIT_HARDFORK_0_10_VERSION;
        FC_ASSERT(STEEMIT_HARDFORK_0_11 ==
                  11, "Invalid hardfork configuration");
        _hardfork_times[STEEMIT_HARDFORK_0_11] = fc::time_point_sec(STEEMIT_HARDFORK_0_11_TIME);
        _hardfork_versions[STEEMIT_HARDFORK_0_11] = STEEMIT_HARDFORK_0_11_VERSION;
        FC_ASSERT(STEEMIT_HARDFORK_0_12 ==
                  12, "Invalid hardfork configuration");
        _hardfork_times[STEEMIT_HARDFORK_0_12] = fc::time_point_sec(STEEMIT_HARDFORK_0_12_TIME);
        _hardfork_versions[STEEMIT_HARDFORK_0_12] = STEEMIT_HARDFORK_0_12_VERSION;
        FC_ASSERT(STEEMIT_HARDFORK_0_13 ==
                  13, "Invalid hardfork configuration");
        _hardfork_times[STEEMIT_HARDFORK_0_13] = fc::time_point_sec(STEEMIT_HARDFORK_0_13_TIME);
        _hardfork_versions[STEEMIT_HARDFORK_0_13] = STEEMIT_HARDFORK_0_13_VERSION;
        FC_ASSERT(STEEMIT_HARDFORK_0_14 ==
                  14, "Invalid hardfork configuration");
        _hardfork_times[STEEMIT_HARDFORK_0_14] = fc::time_point_sec(STEEMIT_HARDFORK_0_14_TIME);
        _hardfork_versions[STEEMIT_HARDFORK_0_14] = STEEMIT_HARDFORK_0_14_VERSION;
        FC_ASSERT(STEEMIT_HARDFORK_0_15 ==
                  15, "Invalid hardfork configuration");
        _hardfork_times[STEEMIT_HARDFORK_0_15] = fc::time_point_sec(STEEMIT_HARDFORK_0_15_TIME);
        _hardfork_versions[STEEMIT_HARDFORK_0_15] = STEEMIT_HARDFORK_0_15_VERSION;
        FC_ASSERT(STEEMIT_HARDFORK_0_16 ==
                  16, "Invalid hardfork configuration");
        _hardfork_times[STEEMIT_HARDFORK_0_16] = fc::time_point_sec(STEEMIT_HARDFORK_0_16_TIME);
        _hardfork_versions[STEEMIT_HARDFORK_0_16] = STEEMIT_HARDFORK_0_16_VERSION;
        FC_ASSERT(STEEMIT_HARDFORK_0_17 ==
                  17, "Invalid hardfork configuration");
        _hardfork_times[STEEMIT_HARDFORK_0_17] = fc::time_point_sec(STEEMIT_HARDFORK_0_17_TIME);
        _hardfork_versions[STEEMIT_HARDFORK_0_17] = STEEMIT_HARDFORK_0_17_VERSION;

        const auto &hardforks = get_hardfork_property_object();
        FC_ASSERT(hardforks.last_hardfork <=
                  STEEMIT_NUM_HARDFORKS, "Chain knows of more hardforks than configuration", ("hardforks.last_hardfork", hardforks.last_hardfork)("STEEMIT_NUM_HARDFORKS", STEEMIT_NUM_HARDFORKS));
        FC_ASSERT(_hardfork_versions[hardforks.last_hardfork] <=
                  STEEMIT_BLOCKCHAIN_VERSION, "Blockchain version is older than last applied hardfork");
        FC_ASSERT(STEEMIT_BLOCKCHAIN_HARDFORK_VERSION ==
                  _hardfork_versions[STEEMIT_NUM_HARDFORKS]);
    }
};
}}
#endif //GOLOS_HARDFORK_PROPERTY_POLICY_HPP
