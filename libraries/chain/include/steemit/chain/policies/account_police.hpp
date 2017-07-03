#ifndef GOLOS_ACCOUNT_POLICE_HPP
#define GOLOS_ACCOUNT_POLICE_HPP
namespace steemit {
namespace chain {

class database_basic;

struct account_read_police {

    account_read_police() = default;

    account_read_police(const account_read_police &) = default;

    account_read_police &operator=(const account_read_police &) = default;

    account_read_police(account_read_police &&) = default;

    account_read_police &operator=(account_read_police &&) = default;

    virtual ~account_read_police() = default;

    account_read_police(database_basic &ref,evaluator_registry<operation>& evaluator_registry_) : evaluator_registry_(evaluator_registry_), references(ref) {
        add_core_index<database_basic,account_index>(ref);
        add_core_index<database_basic,account_authority_index>(ref);
        add_core_index<database_basic,account_bandwidth_index>(ref);
        add_core_index<database_basic,account_recovery_request_index>(ref);
        evaluator_registry.register_evaluator<account_create_evaluator>();
        evaluator_registry.register_evaluator<account_update_evaluator>();
    }


    const account_object &get_account(const account_name_type &name) const {
        try {
            return references.get<account_object, by_name>(name);
        } FC_CAPTURE_AND_RETHROW((name))
    }

    const account_object *find_account(const account_name_type &name) const {
        return references.find<account_object, by_name>(name);
    }


    void clear_null_account_balance() {
        if (!has_hardfork(STEEMIT_HARDFORK_0_14__327)) {
            return;
        }

        const auto &null_account = get_account(STEEMIT_NULL_ACCOUNT);
        asset total_steem(0, STEEM_SYMBOL);
        asset total_sbd(0, SBD_SYMBOL);

        if (null_account.balance.amount > 0) {
            total_steem += null_account.balance;
            references.adjust_balance(null_account, -null_account.balance);
        }

        if (null_account.savings_balance.amount > 0) {
            total_steem += null_account.savings_balance;
            references.adjust_savings_balance(null_account, -null_account.savings_balance);
        }

        if (null_account.sbd_balance.amount > 0) {
            total_sbd += null_account.sbd_balance;
            references.adjust_balance(null_account, -null_account.sbd_balance);
        }

        if (null_account.savings_sbd_balance.amount > 0) {
            total_sbd += null_account.savings_sbd_balance;
            references.adjust_savings_balance(null_account, -null_account.savings_sbd_balance);
        }

        if (null_account.vesting_shares.amount > 0) {
            const auto &gpo = references.get_dynamic_global_properties();
            auto converted_steem = null_account.vesting_shares *
                                   gpo.get_vesting_share_price();

            references.modify(gpo, [&](dynamic_global_property_object &g) {
                g.total_vesting_shares -= null_account.vesting_shares;
                g.total_vesting_fund_steem -= converted_steem;
            });

            references.modify(null_account, [&](account_object &a) {
                a.vesting_shares.amount = 0;
            });

            total_steem += converted_steem;
        }

        if (total_steem.amount > 0) {
            references.adjust_supply(-total_steem);
        }

        if (total_sbd.amount > 0) {
            references.adjust_supply(-total_sbd);
        }
    }

    void adjust_proxied_witness_votes(const account_object &a, share_type delta, int depth) {
        if (a.proxy != STEEMIT_PROXY_TO_SELF_ACCOUNT) {
            /// nested proxies are not supported, vote will not propagate
            if (depth >= STEEMIT_MAX_PROXY_RECURSION_DEPTH) {
                return;
            }

            const auto &proxy = get_account(a.proxy);

            references.modify(proxy, [&](account_object &a) {
                a.proxied_vsf_votes[depth] += delta;
            });

            adjust_proxied_witness_votes(proxy, delta, depth + 1);
        } else {
            references.adjust_witness_votes(a, delta);
        }
    }

    void adjust_proxied_witness_votes(
            const account_object &a,
            const std::array<share_type,
            STEEMIT_MAX_PROXY_RECURSION_DEPTH + 1> &delta,
            int depth) {
        if (a.proxy != STEEMIT_PROXY_TO_SELF_ACCOUNT) {
            /// nested proxies are not supported, vote will not propagate
            if (depth >= STEEMIT_MAX_PROXY_RECURSION_DEPTH) {
                return;
            }

            const auto &proxy = get_account(a.proxy);

            references.modify(proxy, [&](account_object &a) {
                for (int i = STEEMIT_MAX_PROXY_RECURSION_DEPTH - depth - 1;
                     i >= 0; --i) {
                    a.proxied_vsf_votes[i + depth] += delta[i];
                }
            });

            adjust_proxied_witness_votes(proxy, delta, depth + 1);
        } else {
            share_type total_delta = 0;
            for (int i = STEEMIT_MAX_PROXY_RECURSION_DEPTH - depth;
                 i >= 0; --i) {
                total_delta += delta[i];
            }
            references.adjust_witness_votes(a, total_delta);
        }
    }

    asset get_balance(const string &aname, asset_symbol_type symbol) const {
        return get_balance(get_account(aname), symbol);
    }

    share_type cashout_comment_helper(utilities::comment_reward_context &ctx, const comment_object &comment) {
        try {
            share_type claimed_reward = 0;

            if (comment.net_rshares > 0) {
                fill_comment_reward_context_local_state(ctx, comment);

                const share_type reward = has_hardfork(STEEMIT_HARDFORK_0_17__86) ? utilities::get_rshare_reward(ctx, get_reward_fund(comment)) : utilities::get_rshare_reward(ctx);
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
                        auto vest_created = create_vesting(get_account(b.account), benefactor_tokens);
                        references.push_virtual_operation(comment_benefactor_reward_operation(b.account, comment.author, to_string(comment.permlink), vest_created));
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
                                                        asset(vesting_steem, STEEM_SYMBOL)), to_sbd(asset(curation_tokens, STEEM_SYMBOL)), to_sbd(asset(total_beneficiary, STEEM_SYMBOL)));


                    /*if( sbd_created.symbol == SBD_SYMBOL )
                       adjust_total_payout( comment, sbd_created + to_sbd( asset( vesting_steem, STEEM_SYMBOL ) ), to_sbd( asset( reward_tokens.to_uint64() - author_tokens, STEEM_SYMBOL ) ) );
                    else
                       adjust_total_payout( comment, to_sbd( asset( vesting_steem + sbd_steem, STEEM_SYMBOL ) ), to_sbd( asset( reward_tokens.to_uint64() - author_tokens, STEEM_SYMBOL ) ) );
                       */


                    references.push_virtual_operation(author_reward_operation(comment.author, to_string(comment.permlink), sbd_payout.first, sbd_payout.second, vest_created));
                    references.push_virtual_operation(comment_reward_operation(comment.author, to_string(comment.permlink), to_sbd(asset(claimed_reward, STEEM_SYMBOL))));

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
                    references.adjust_rshares2(comment, utilities::calculate_vshares(comment.net_rshares.value), 0);
                }

                references.modify(references.get_dynamic_global_properties(), [&](dynamic_global_property_object &p) {
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

            references.push_virtual_operation(comment_payout_update_operation(comment.author, to_string(comment.permlink)));

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
        } FC_CAPTURE_AND_RETHROW((comment))
    }


    /**
 *  Overall the network has an inflation rate of 102% of virtual steem per year
 *  90% of inflation is directed to vesting shares
 *  10% of inflation is directed to subjective proof of work voting
 *  1% of inflation is directed to liquidity providers
 *  1% of inflation is directed to block producers
 *
 *  This method pays out vesting and reward shares every block, and liquidity shares once per day.
 *  This method does not pay out witnesses.
 */
    void process_funds() {
        const auto &props = references.get_dynamic_global_properties();
        const auto &wso = references.get_witness_schedule_object();

        if (has_hardfork(STEEMIT_HARDFORK_0_16__551)) {
            /**
   * At block 7,000,000 have a 9.5% instantaneous inflation rate, decreasing to 0.95% at a rate of 0.01%
   * every 250k blocks. This narrowing will take approximately 20.5 years and will complete on block 220,750,000
   */
            int64_t start_inflation_rate = int64_t(STEEMIT_INFLATION_RATE_START_PERCENT);
            int64_t inflation_rate_adjustment = int64_t(
                    head_block_num() / STEEMIT_INFLATION_NARROWING_PERIOD);
            int64_t inflation_rate_floor = int64_t(STEEMIT_INFLATION_RATE_STOP_PERCENT);

            // below subtraction cannot underflow int64_t because inflation_rate_adjustment is <2^32
            int64_t current_inflation_rate = std::max(start_inflation_rate -
                                                      inflation_rate_adjustment, inflation_rate_floor);

            auto new_steem =
                    (props.virtual_supply.amount * current_inflation_rate) /
                    (int64_t(STEEMIT_100_PERCENT) *
                     int64_t(STEEMIT_BLOCKS_PER_YEAR));
            auto content_reward =
                    (new_steem * STEEMIT_CONTENT_REWARD_PERCENT) /
                    STEEMIT_100_PERCENT;
            if (has_hardfork(STEEMIT_HARDFORK_0_17__86)) {
                content_reward = pay_reward_funds(content_reward);
            } /// 75% to content creator
            auto vesting_reward =
                    (new_steem * STEEMIT_VESTING_FUND_PERCENT) /
                    STEEMIT_100_PERCENT; /// 15% to vesting fund
            auto witness_reward = new_steem - content_reward -
                                  vesting_reward; /// Remaining 10% to witness pay

            const auto &cwit = references.get_witness(props.current_witness);
            witness_reward *= STEEMIT_MAX_WITNESSES;

            if (cwit.schedule == witness_object::timeshare) {
                witness_reward *= wso.timeshare_weight;
            } else if (cwit.schedule == witness_object::miner) {
                witness_reward *= wso.miner_weight;
            } else if (cwit.schedule == witness_object::top19) {
                witness_reward *= wso.top19_weight;
            } else
                wlog("Encountered unknown witness type for witness: ${w}", ("w", cwit.owner));

            witness_reward /= wso.witness_pay_normalization_factor;

            new_steem = content_reward + vesting_reward + witness_reward;

            references.modify(props, [&](dynamic_global_property_object &p) {
                p.total_vesting_fund_steem += asset(vesting_reward, STEEM_SYMBOL);
                if (!has_hardfork(STEEMIT_HARDFORK_0_17__86)) {
                    p.total_reward_fund_steem += asset(content_reward, STEEM_SYMBOL);
                }
                p.current_supply += asset(new_steem, STEEM_SYMBOL);
                p.virtual_supply += asset(new_steem, STEEM_SYMBOL);

            });

            create_vesting(get_account(cwit.owner), asset(witness_reward, STEEM_SYMBOL));
        } else {
            auto content_reward = get_content_reward();
            auto curate_reward = get_curation_reward();
            auto witness_pay = get_producer_reward();
            auto vesting_reward =
                    content_reward + curate_reward + witness_pay;

            content_reward = content_reward + curate_reward;

            if (props.head_block_number < STEEMIT_START_VESTING_BLOCK) {
                vesting_reward.amount = 0;
            } else {
                vesting_reward.amount.value *= 9;
            }

            references.modify(props, [&](dynamic_global_property_object &p) {
                p.total_vesting_fund_steem += vesting_reward;
                p.total_reward_fund_steem += content_reward;
                p.current_supply +=
                        content_reward + witness_pay + vesting_reward;
                p.virtual_supply +=
                        content_reward + witness_pay + vesting_reward;
            });
        }
    }

    void process_savings_withdraws() {
        const auto &idx = get_index<savings_withdraw_index>().indices().get<by_complete_from_rid>();
        auto itr = idx.begin();
        while (itr != idx.end()) {
            if (itr->complete > head_block_time()) {
                break;
            }
            references.adjust_balance(get_account(itr->to), itr->amount);

            references.modify(get_account(itr->from), [&](account_object &a) {
                a.savings_withdraw_requests--;
            });

            references.push_virtual_operation(fill_transfer_from_savings_operation(itr->from, itr->to, itr->amount, itr->request_id, to_string(itr->memo)));

            remove(*itr);
            itr = idx.begin();
        }
    }


    asset get_producer_reward() {
        const auto &props = references.get_dynamic_global_properties();
        static_assert(STEEMIT_BLOCK_INTERVAL ==
                      3, "this code assumes a 3-second time interval");
        asset percent(protocol::calc_percent_reward_per_block<STEEMIT_PRODUCER_APR_PERCENT>(props.virtual_supply.amount), STEEM_SYMBOL);

        const auto &witness_account = get_account(props.current_witness);

        if (has_hardfork(STEEMIT_HARDFORK_0_16)) {
            auto pay = std::max(percent, STEEMIT_MIN_PRODUCER_REWARD);

            /// pay witness in vesting shares
            if (props.head_block_number >=
                STEEMIT_START_MINER_VOTING_BLOCK ||
                (witness_account.vesting_shares.amount.value == 0)) {
                // const auto& witness_obj = get_witness( props.current_witness );
                create_vesting(witness_account, pay);
            } else {
                references.modify(get_account(witness_account.name), [&](account_object &a) {
                    a.balance += pay;
                });
            }

            return pay;
        } else {
            auto pay = std::max(percent, STEEMIT_MIN_PRODUCER_REWARD_PRE_HF16);

            /// pay witness in vesting shares
            if (props.head_block_number >=
                STEEMIT_START_MINER_VOTING_BLOCK ||
                (witness_account.vesting_shares.amount.value == 0)) {
                // const auto& witness_obj = get_witness( props.current_witness );
                create_vesting(witness_account, pay);
            } else {
                references.modify(get_account(witness_account.name), [&](account_object &a) {
                    a.balance += pay;
                });
            }

            return pay;
        }
    }



    void account_recovery_processing() {
        // Clear expired recovery requests
        const auto &rec_req_idx = get_index<account_recovery_request_index>().indices().get<by_expiration>();
        auto rec_req = rec_req_idx.begin();

        while (rec_req != rec_req_idx.end() &&
               rec_req->expires <= head_block_time()) {
            remove(*rec_req);
            rec_req = rec_req_idx.begin();
        }

        // Clear invalid historical authorities
        const auto &hist_idx = get_index<owner_authority_history_index>().indices(); //by id
        auto hist = hist_idx.begin();

        while (hist != hist_idx.end() && time_point_sec(
                hist->last_valid_time +
                STEEMIT_OWNER_AUTH_RECOVERY_PERIOD) < head_block_time()) {
            remove(*hist);
            hist = hist_idx.begin();
        }

        // Apply effective recovery_account changes
        const auto &change_req_idx = get_index<change_recovery_account_request_index>().indices().get<by_effective_date>();
        auto change_req = change_req_idx.begin();

        while (change_req != change_req_idx.end() &&
               change_req->effective_on <= head_block_time()) {
            references.modify(get_account(change_req->account_to_recover), [&](account_object &a) {
                a.recovery_account = change_req->recovery_account;
            });

            remove(*change_req);
            change_req = change_req_idx.begin();
        }
    }


    void expire_escrow_ratification() {
        const auto &escrow_idx = get_index<escrow_index>().indices().get<by_ratification_deadline>();
        auto escrow_itr = escrow_idx.lower_bound(false);

        while (escrow_itr != escrow_idx.end() &&
               !escrow_itr->is_approved() &&
               escrow_itr->ratification_deadline <= head_block_time()) {
            const auto &old_escrow = *escrow_itr;
            ++escrow_itr;

            const auto &from_account = get_account(old_escrow.from);
            references.adjust_balance(from_account, old_escrow.steem_balance);
            references.adjust_balance(from_account, old_escrow.sbd_balance);
            references.adjust_balance(from_account, old_escrow.pending_fee);

            remove(old_escrow);
        }
    }




    /**
*  Iterates over all conversion requests with a conversion date before
*  the head block time and then converts them to/from steem/sbd at the
*  current median price feed history price times the premium
*/
    void process_conversions() {
        auto now = head_block_time();
        const auto &request_by_date = get_index<convert_request_index>().indices().get<by_conversion_date>();
        auto itr = request_by_date.begin();

        const auto &fhistory = references.get_feed_history();
        if (fhistory.current_median_history.is_null()) {
            return;
        }

        asset net_sbd(0, SBD_SYMBOL);
        asset net_steem(0, STEEM_SYMBOL);

        while (itr != request_by_date.end() &&
               itr->conversion_date <= now) {
            const auto &user = get_account(itr->owner);
            auto amount_to_issue =
                    itr->amount * fhistory.current_median_history;

            references.adjust_balance(user, amount_to_issue);

            net_sbd += itr->amount;
            net_steem += amount_to_issue;

            references.push_virtual_operation(fill_convert_request_operation(user.name, itr->requestid, itr->amount, amount_to_issue));

            remove(*itr);
            itr = request_by_date.begin();
        }

        const auto &props = references.get_dynamic_global_properties();
        references.modify(props, [&](dynamic_global_property_object &p) {
            p.current_supply += net_steem;
            p.current_sbd_supply -= net_sbd;
            p.virtual_supply += net_steem;
            p.virtual_supply -=
                    net_sbd * references.get_feed_history().current_median_history;
        });
    }


    int match(const limit_order_object &new_order, const limit_order_object &old_order, const price &match_price) {
        assert(new_order.sell_price.quote.symbol ==
               old_order.sell_price.base.symbol);
        assert(new_order.sell_price.base.symbol ==
               old_order.sell_price.quote.symbol);
        assert(new_order.for_sale > 0 && old_order.for_sale > 0);
        assert(match_price.quote.symbol ==
               new_order.sell_price.base.symbol);
        assert(match_price.base.symbol == old_order.sell_price.base.symbol);

        auto new_order_for_sale = new_order.amount_for_sale();
        auto old_order_for_sale = old_order.amount_for_sale();

        asset new_order_pays, new_order_receives, old_order_pays, old_order_receives;

        if (new_order_for_sale <= old_order_for_sale * match_price) {
            old_order_receives = new_order_for_sale;
            new_order_receives = new_order_for_sale * match_price;
        } else {
            //This line once read: assert( old_order_for_sale < new_order_for_sale * match_price );
            //This assert is not always true -- see trade_amount_equals_zero in operation_tests.cpp
            //Although new_order_for_sale is greater than old_order_for_sale * match_price, old_order_for_sale == new_order_for_sale * match_price
            //Removing the assert seems to be safe -- apparently no asset is created or destroyed.
            new_order_receives = old_order_for_sale;
            old_order_receives = old_order_for_sale * match_price;
        }

        old_order_pays = new_order_receives;
        new_order_pays = old_order_receives;

        assert(new_order_pays == new_order.amount_for_sale() ||
               old_order_pays == old_order.amount_for_sale());

        auto age = head_block_time() - old_order.created;
        if (!has_hardfork(STEEMIT_HARDFORK_0_12__178) &&
            ((age >= STEEMIT_MIN_LIQUIDITY_REWARD_PERIOD_SEC &&
              !has_hardfork(STEEMIT_HARDFORK_0_10__149)) ||
             (age >= STEEMIT_MIN_LIQUIDITY_REWARD_PERIOD_SEC_HF10 &&
              has_hardfork(STEEMIT_HARDFORK_0_10__149)))) {
            if (old_order_receives.symbol == STEEM_SYMBOL) {
                references.adjust_liquidity_reward(get_account(old_order.seller), old_order_receives, false);
                references.adjust_liquidity_reward(get_account(new_order.seller), -old_order_receives, false);
            } else {
                references.adjust_liquidity_reward(get_account(old_order.seller), new_order_receives, true);
                references.adjust_liquidity_reward(get_account(new_order.seller), -new_order_receives, true);
            }
        }

        references.push_virtual_operation(fill_order_operation(new_order.seller, new_order.orderid, new_order_pays, old_order.seller, old_order.orderid, old_order_pays));

        int result = 0;
        result |= fill_order(new_order, new_order_pays, new_order_receives);
        result |= fill_order(old_order, old_order_pays, old_order_receives)
                << 1;
        assert(result != 0);
        return result;
    }


    bool fill_order(const limit_order_object &order, const asset &pays, const asset &receives) {
        try {
            FC_ASSERT(order.amount_for_sale().symbol == pays.symbol);
            FC_ASSERT(pays.symbol != receives.symbol);

            const account_object &seller = get_account(order.seller);

            references.adjust_balance(seller, receives);

            if (pays == order.amount_for_sale()) {
                remove(order);
                return true;
            } else {
                references.modify(order, [&](limit_order_object &b) {
                    b.for_sale -= pays.amount;
                });
                /**
      *  There are times when the AMOUNT_FOR_SALE * SALE_PRICE == 0 which means that we
      *  have hit the limit where the seller is asking for nothing in return.  When this
      *  happens we must refund any balance back to the seller, it is too small to be
      *  sold at the sale price.
      */
                if (order.amount_to_receive().amount == 0) {
                    cancel_order(order);
                    return true;
                }
                return false;
            }
        }
        FC_CAPTURE_AND_RETHROW((order)(pays)(receives))
    }

    void cancel_order(const limit_order_object &order) {
        references.adjust_balance(get_account(order.seller), order.amount_for_sale());
        references.remove(order);
    }


    void clear_expired_delegations() {
        auto now = head_block_time();
        const auto &delegations_by_exp = get_index<vesting_delegation_expiration_index, by_expiration>();
        auto itr = delegations_by_exp.begin();
        while (itr != delegations_by_exp.end() && itr->expiration < now) {
            references.modify(get_account(itr->delegator), [&](account_object &a) {
                a.delegated_vesting_shares -= itr->vesting_shares;
            });

            references.push_virtual_operation(return_vesting_delegation_operation(itr->delegator, itr->vesting_shares));

            remove(*itr);
            itr = delegations_by_exp.begin();
        }
    }

protected:
    database_basic &references;

    evaluator_registry<operation>& evaluator_registry_;

};

}}
#endif
