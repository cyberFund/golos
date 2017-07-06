#ifndef GOLOS_DYNAMIC_GLOBAL_PROPERTY_POLICY_HPP
#define GOLOS_DYNAMIC_GLOBAL_PROPERTY_POLICY_HPP
namespace steemit {
namespace chain {
struct dynamic_global_property_policy {

    dynamic_global_property_policy() = default;

    dynamic_global_property_policy(const behaviour_based_policy &) = default;

    dynamic_global_property_policy &operator=(const behaviour_based_policy &) = default;

    dynamic_global_property_policy(behaviour_based_policy &&) = default;

    dynamic_global_property_policy &operator=(behaviour_based_policy &&) = default;

    virtual ~dynamic_global_property_policy() = default;

    dynamic_global_property_policy(database_basic &ref, evaluator_registry <operation> &evaluator_registry_) : references(ref) {

    }

    const dynamic_global_property_object &get_dynamic_global_properties() const {
        try {
            return get<dynamic_global_property_object>();
        }
        FC_CAPTURE_AND_RETHROW()

    }

    time_point_sec head_block_time() const {
        return get_dynamic_global_properties().time;
    }

    void update_global_dynamic_data(const signed_block &b) {
        try {
            auto block_size = fc::raw::pack_size(b);
            const dynamic_global_property_object &_dgp =
                    get_dynamic_global_properties();

            uint32_t missed_blocks = 0;
            if (head_block_time() != fc::time_point_sec()) {
                missed_blocks = get_slot_at_time(b.timestamp);
                assert(missed_blocks != 0);
                missed_blocks--;
                for (uint32_t i = 0; i < missed_blocks; ++i) {
                    const auto &witness_missed = get_witness(get_scheduled_witness(
                            i + 1));
                    if (witness_missed.owner != b.witness) {
                        modify(witness_missed, [&](witness_object &w) {
                            w.total_missed++;
                            if (has_hardfork(STEEMIT_HARDFORK_0_14__278)) {
                                if (head_block_num() -
                                    w.last_confirmed_block_num >
                                    STEEMIT_BLOCKS_PER_DAY) {
                                    w.signing_key = public_key_type();
                                    push_virtual_operation(shutdown_witness_operation(w.owner));
                                }
                            }
                        });
                    }
                }
            }

            // dynamic global properties updating
            modify(_dgp, [&](dynamic_global_property_object &dgp) {
                // This is constant time assuming 100% participation. It is O(B) otherwise (B = Num blocks between update)
                for (uint32_t i = 0; i < missed_blocks + 1; i++) {
                    dgp.participation_count -= dgp.recent_slots_filled.hi &
                                               0x8000000000000000ULL ? 1
                                                                     : 0;
                    dgp.recent_slots_filled =
                            (dgp.recent_slots_filled << 1) +
                            (i == 0 ? 1 : 0);
                    dgp.participation_count += (i == 0 ? 1 : 0);
                }

                dgp.head_block_number = b.block_num();
                dgp.head_block_id = b.id();
                dgp.time = b.timestamp;
                dgp.current_aslot += missed_blocks + 1;
                dgp.average_block_size =
                        (99 * dgp.average_block_size + block_size) / 100;

                /**
   *  About once per minute the average network use is consulted and used to
   *  adjust the reserve ratio. Anything above 50% usage reduces the ratio by
   *  half which should instantly bring the network from 50% to 25% use unless
   *  the demand comes from users who have surplus capacity. In other words,
   *  a 50% reduction in reserve ratio does not result in a 50% reduction in usage,
   *  it will only impact users who where attempting to use more than 50% of their
   *  capacity.
   *
   *  When the reserve ratio is at its max (10,000) a 50% reduction will take 3 to
   *  4 days to return back to maximum.  When it is at its minimum it will return
   *  back to its prior level in just a few minutes.
   *
   *  If the network reserve ratio falls under 100 then it is probably time to
   *  increase the capacity of the network.
   */
                if (dgp.head_block_number % 20 == 0) {
                    if ((!has_hardfork(STEEMIT_HARDFORK_0_12__179) &&
                         dgp.average_block_size >
                         dgp.maximum_block_size / 2) ||
                        (has_hardfork(STEEMIT_HARDFORK_0_12__179) &&
                         dgp.average_block_size >
                         dgp.maximum_block_size / 4)) {
                        dgp.current_reserve_ratio /= 2; /// exponential back up
                    } else { /// linear growth... not much fine grain control near full capacity
                        dgp.current_reserve_ratio++;
                    }

                    if (has_hardfork(STEEMIT_HARDFORK_0_2) &&
                        dgp.current_reserve_ratio >
                        STEEMIT_MAX_RESERVE_RATIO) {
                        dgp.current_reserve_ratio = STEEMIT_MAX_RESERVE_RATIO;
                    }
                }
                dgp.max_virtual_bandwidth = (dgp.maximum_block_size *
                                             dgp.current_reserve_ratio *
                                             STEEMIT_BANDWIDTH_PRECISION *
                                             STEEMIT_BANDWIDTH_AVERAGE_WINDOW_SECONDS) /
                                            STEEMIT_BLOCK_INTERVAL;
            });

            if (!(get_node_properties().skip_flags &
                  skip_undo_history_check)) {
                STEEMIT_ASSERT(_dgp.head_block_number -
                               _dgp.last_irreversible_block_num <
                               STEEMIT_MAX_UNDO_HISTORY, undo_database_exception,
                               "The database does not have enough undo history to support a blockchain with so many missed blocks. "
                                       "Please add a checkpoint if you would like to continue applying blocks beyond this point.",
                               ("last_irreversible_block_num", _dgp.last_irreversible_block_num)("head", _dgp.head_block_number)
                                       ("max_undo", STEEMIT_MAX_UNDO_HISTORY));
            }
        } FC_CAPTURE_AND_RETHROW()
    }

    void update_virtual_supply() {
        try {
            modify(get_dynamic_global_properties(), [&](dynamic_global_property_object &dgp) {
                dgp.virtual_supply = dgp.current_supply
                                     +
                                     (get_feed_history().current_median_history.is_null()
                                      ? asset(0, STEEM_SYMBOL) :
                                      dgp.current_sbd_supply *
                                      get_feed_history().current_median_history);

                auto median_price = get_feed_history().current_median_history;

                if (!median_price.is_null() &&
                    has_hardfork(STEEMIT_HARDFORK_0_14__230)) {
                    auto percent_sbd = uint16_t((
                                                        (fc::uint128_t((dgp.current_sbd_supply *
                                                                        get_feed_history().current_median_history).amount.value) *
                                                         STEEMIT_100_PERCENT)
                                                        / dgp.virtual_supply.amount.value).to_uint64());

                    if (percent_sbd <= STEEMIT_SBD_START_PERCENT) {
                        dgp.sbd_print_rate = STEEMIT_100_PERCENT;
                    } else if (percent_sbd >= STEEMIT_SBD_STOP_PERCENT) {
                        dgp.sbd_print_rate = 0;
                    } else {
                        dgp.sbd_print_rate =
                                ((STEEMIT_SBD_STOP_PERCENT - percent_sbd) *
                                 STEEMIT_100_PERCENT) /
                                (STEEMIT_SBD_STOP_PERCENT -
                                 STEEMIT_SBD_START_PERCENT);
                    }
                }
            });
        } FC_CAPTURE_AND_RETHROW()
    }

    /**
 * Verifies all supply invariantes check out
 */
    void database_basic::validate_invariants() const {
        try {
            const auto &account_idx = get_index<account_index>().indices().get<by_name>();
            asset total_supply = asset(0, STEEM_SYMBOL);
            asset total_sbd = asset(0, SBD_SYMBOL);
            asset total_vesting = asset(0, VESTS_SYMBOL);
            share_type total_vsf_votes = share_type(0);

            auto gpo = get_dynamic_global_properties();

            /// verify no witness has too many votes
            const auto &witness_idx = get_index<witness_index>().indices();
            for (auto itr = witness_idx.begin();
                 itr != witness_idx.end(); ++itr)
                FC_ASSERT(itr->votes <
                          gpo.total_vesting_shares.amount, "", ("itr", *itr));

            for (auto itr = account_idx.begin();
                 itr != account_idx.end(); ++itr) {
                total_supply += itr->balance;
                total_supply += itr->savings_balance;
                total_sbd += itr->sbd_balance;
                total_sbd += itr->savings_sbd_balance;
                total_vesting += itr->vesting_shares;
                total_vsf_votes += (itr->proxy ==
                                    STEEMIT_PROXY_TO_SELF_ACCOUNT ?
                                    itr->witness_vote_weight() :
                                    (STEEMIT_MAX_PROXY_RECURSION_DEPTH > 0 ?
                                     itr->proxied_vsf_votes[
                                             STEEMIT_MAX_PROXY_RECURSION_DEPTH -
                                             1] :
                                     itr->vesting_shares.amount));
            }

            const auto &convert_request_idx = get_index<convert_request_index>().indices();

            for (auto itr = convert_request_idx.begin();
                 itr != convert_request_idx.end(); ++itr) {
                if (itr->amount.symbol == STEEM_SYMBOL) {
                    total_supply += itr->amount;
                } else if (itr->amount.symbol == SBD_SYMBOL) {
                    total_sbd += itr->amount;
                } else
                    FC_ASSERT(false, "Encountered illegal symbol in convert_request_object");
            }

            const auto &limit_order_idx = get_index<limit_order_index>().indices();

            for (auto itr = limit_order_idx.begin();
                 itr != limit_order_idx.end(); ++itr) {
                if (itr->sell_price.base.symbol == STEEM_SYMBOL) {
                    total_supply += asset(itr->for_sale, STEEM_SYMBOL);
                } else if (itr->sell_price.base.symbol == SBD_SYMBOL) {
                    total_sbd += asset(itr->for_sale, SBD_SYMBOL);
                }
            }

            const auto &escrow_idx = get_index<escrow_index>().indices().get<by_id>();

            for (auto itr = escrow_idx.begin();
                 itr != escrow_idx.end(); ++itr) {
                total_supply += itr->steem_balance;
                total_sbd += itr->sbd_balance;

                if (itr->pending_fee.symbol == STEEM_SYMBOL) {
                    total_supply += itr->pending_fee;
                } else if (itr->pending_fee.symbol == SBD_SYMBOL) {
                    total_sbd += itr->pending_fee;
                } else
                    FC_ASSERT(false, "found escrow pending fee that is not SBD or STEEM");
            }

            const auto &savings_withdraw_idx = get_index<savings_withdraw_index>().indices().get<by_id>();

            for (auto itr = savings_withdraw_idx.begin();
                 itr != savings_withdraw_idx.end(); ++itr) {
                if (itr->amount.symbol == STEEM_SYMBOL) {
                    total_supply += itr->amount;
                } else if (itr->amount.symbol == SBD_SYMBOL) {
                    total_sbd += itr->amount;
                } else
                    FC_ASSERT(false, "found savings withdraw that is not SBD or STEEM");
            }

            fc::uint128_t total_rshares2;
            fc::uint128_t total_children_rshares2;

            const auto &comment_idx = get_index<comment_index>().indices();

            for (auto itr = comment_idx.begin();
                 itr != comment_idx.end(); ++itr) {
                if (itr->net_rshares.value > 0) {
                    auto delta = utilities::calculate_vshares(itr->net_rshares.value);
                    total_rshares2 += delta;
                }
                if (itr->parent_author == STEEMIT_ROOT_POST_PARENT) {
                    total_children_rshares2 += itr->children_rshares2;
                }
            }

            const auto &reward_idx = get_index<reward_fund_index, by_id>();

            for (auto itr = reward_idx.begin();
                 itr != reward_idx.end(); ++itr) {
                total_supply += itr->reward_balance;
            }

            total_supply += gpo.total_vesting_fund_steem +
                            gpo.total_reward_fund_steem;

            FC_ASSERT(gpo.current_supply ==
                      total_supply, "", ("gpo.current_supply", gpo.current_supply)("total_supply", total_supply));
            FC_ASSERT(gpo.current_sbd_supply ==
                      total_sbd, "", ("gpo.current_sbd_supply", gpo.current_sbd_supply)("total_sbd", total_sbd));
            FC_ASSERT(gpo.total_vesting_shares ==
                      total_vesting, "", ("gpo.total_vesting_shares", gpo.total_vesting_shares)("total_vesting", total_vesting));
            FC_ASSERT(gpo.total_vesting_shares.amount ==
                      total_vsf_votes, "", ("total_vesting_shares", gpo.total_vesting_shares)("total_vsf_votes", total_vsf_votes));

            FC_ASSERT(gpo.virtual_supply >= gpo.current_supply);
            if (!get_feed_history().current_median_history.is_null()) {
                FC_ASSERT(gpo.current_sbd_supply *
                          get_feed_history().current_median_history +
                          gpo.current_supply
                          ==
                          gpo.virtual_supply, "", ("gpo.current_sbd_supply", gpo.current_sbd_supply)("get_feed_history().current_median_history", get_feed_history().current_median_history)("gpo.current_supply", gpo.current_supply)("gpo.virtual_supply", gpo.virtual_supply));
            }
        }
        FC_CAPTURE_LOG_AND_RETHROW((head_block_num()));
    }


protected:
    database_basic &references;

};
}}
#endif //GOLOS_DYNAMIC_GLOBAL_PROPERTY_POLICY_HPP
