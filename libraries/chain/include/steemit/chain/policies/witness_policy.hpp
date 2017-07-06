#ifndef GOLOS_WITNESS_POLICY_HPP
#define GOLOS_WITNESS_POLICY_HPP
namespace steemit {
namespace chain {
struct witness_policy {

    witness_policy() = default;

    witness_policy(const witness_policy &) = default;

    witness_policy &operator=(const witness_policy &) = default;

    witness_policy(witness_policy &&) = default;

    witness_policy &operator=(witness_policy &&) = default;

    virtual ~witness_policy() = default;

    witness_policy(database_basic &ref, evaluator_registry <operation> &evaluator_registry_) : references(ref) {
    }

    const witness_object &get_witness(const account_name_type &name) const {
        try {
            return get<witness_object, by_name>(name);
        } FC_CAPTURE_AND_RETHROW((name))
    }

    const witness_object *find_witness(const account_name_type &name) const {
        return find<witness_object, by_name>(name);
    }


    uint32_t witness_participation_rate() const {
        const dynamic_global_property_object &dpo = get_dynamic_global_properties();
        return uint64_t(STEEMIT_100_PERCENT) *
               dpo.recent_slots_filled.popcount() / 128;
    }

    void retally_witness_votes() {
        const auto &witness_idx = get_index<witness_index>().indices();

        // Clear all witness votes
        for (auto itr = witness_idx.begin();
             itr != witness_idx.end(); ++itr) {
            modify(*itr, [&](witness_object &w) {
                w.votes = 0;
                w.virtual_position = 0;
            });
        }

        const auto &account_idx = get_index<account_index>().indices();

        // Apply all existing votes by account
        for (auto itr = account_idx.begin();
             itr != account_idx.end(); ++itr) {
            if (itr->proxy != STEEMIT_PROXY_TO_SELF_ACCOUNT) {
                continue;
            }

            const auto &a = *itr;

            const auto &vidx = get_index<witness_vote_index>().indices().get<by_account_witness>();
            auto wit_itr = vidx.lower_bound(boost::make_tuple(a.id, witness_id_type()));
            while (wit_itr != vidx.end() && wit_itr->account == a.id) {
                adjust_witness_vote(get(wit_itr->witness), a.witness_vote_weight());
                ++wit_itr;
            }
        }
    }


    void retally_witness_vote_counts(bool force) {
        const auto &account_idx = get_index<account_index>().indices();

        // Check all existing votes by account
        for (auto itr = account_idx.begin();
             itr != account_idx.end(); ++itr) {
            const auto &a = *itr;
            uint16_t witnesses_voted_for = 0;
            if (force || (a.proxy != STEEMIT_PROXY_TO_SELF_ACCOUNT)) {
                const auto &vidx = get_index<witness_vote_index>().indices().get<by_account_witness>();
                auto wit_itr = vidx.lower_bound(boost::make_tuple(a.id, witness_id_type()));
                while (wit_itr != vidx.end() && wit_itr->account == a.id) {
                    ++witnesses_voted_for;
                    ++wit_itr;
                }
            }
            if (a.witnesses_voted_for != witnesses_voted_for) {
                modify(a, [&](account_object &account) {
                    account.witnesses_voted_for = witnesses_voted_for;
                });
            }
        }
    }

    void process_decline_voting_rights() {
        const auto &request_idx = get_index<decline_voting_rights_request_index>().indices().get<by_effective_date>();
        auto itr = request_idx.begin();

        while (itr != request_idx.end() &&
               itr->effective_date <= head_block_time()) {
            const auto &account = get(itr->account);

            /// remove all current votes
            std::array<share_type,
                    STEEMIT_MAX_PROXY_RECURSION_DEPTH + 1> delta;
            delta[0] = -account.vesting_shares.amount;
            for (int i = 0; i < STEEMIT_MAX_PROXY_RECURSION_DEPTH; ++i) {
                delta[i + 1] = -account.proxied_vsf_votes[i];
            }
            adjust_proxied_witness_votes(account, delta);

            clear_witness_votes(account);

            modify(get(itr->account), [&](account_object &a) {
                a.can_vote = false;
                a.proxy = STEEMIT_PROXY_TO_SELF_ACCOUNT;
            });

            remove(*itr);
            itr = request_idx.begin();
        }
    }


/**
 * @param to_account - the account to receive the new vesting shares
 * @param STEEM - STEEM to be converted to vesting shares
 */
    asset create_vesting(const account_object &to_account, asset steem) {
        try {
            const auto &cprops = get_dynamic_global_properties();

            /**
   *  The ratio of total_vesting_shares / total_vesting_fund_steem should not
   *  change as the result of the user adding funds
   *
   *  V / C  = (V+Vn) / (C+Cn)
   *
   *  Simplifies to Vn = (V * Cn ) / C
   *
   *  If Cn equals o.amount, then we must solve for Vn to know how many new vesting shares
   *  the user should receive.
   *
   *  128 bit math is requred due to multiplying of 64 bit numbers. This is done in asset and price.
   */
            asset new_vesting = steem * cprops.get_vesting_share_price();

            modify(to_account, [&](account_object &to) {
                to.vesting_shares += new_vesting;
            });

            modify(cprops, [&](dynamic_global_property_object &props) {
                props.total_vesting_fund_steem += steem;
                props.total_vesting_shares += new_vesting;
            });

            adjust_proxied_witness_votes(to_account, new_vesting.amount);

            return new_vesting;
        }
        FC_CAPTURE_AND_RETHROW((to_account.name)(steem))
    }


    void adjust_witness_vote(const witness_object &witness, share_type delta) {
        const witness_schedule_object &wso = get_witness_schedule_object();
        modify(witness, [&](witness_object &w) {
            auto delta_pos = w.votes.value * (wso.current_virtual_time -
                                              w.virtual_last_update);
            w.virtual_position += delta_pos;

            w.virtual_last_update = wso.current_virtual_time;
            w.votes += delta;
            FC_ASSERT(w.votes <=
                      get_dynamic_global_properties().total_vesting_shares.amount, "", ("w.votes", w.votes)("props", get_dynamic_global_properties().total_vesting_shares));

            if (has_hardfork(STEEMIT_HARDFORK_0_2)) {
                w.virtual_scheduled_time = w.virtual_last_update +
                                           (VIRTUAL_SCHEDULE_LAP_LENGTH2 -
                                            w.virtual_position) /
                                           (w.votes.value + 1);
            } else {
                w.virtual_scheduled_time = w.virtual_last_update +
                                           (VIRTUAL_SCHEDULE_LAP_LENGTH -
                                            w.virtual_position) /
                                           (w.votes.value + 1);
            }

            /** witnesses with a low number of votes could overflow the time field and end up with a scheduled time in the past */
            if (has_hardfork(STEEMIT_HARDFORK_0_4)) {
                if (w.virtual_scheduled_time < wso.current_virtual_time) {
                    w.virtual_scheduled_time = fc::uint128::max_value();
                }
            }
        });
    }

    void clear_witness_votes(const account_object &a) {
        const auto &vidx = get_index<witness_vote_index>().indices().get<by_account_witness>();
        auto itr = vidx.lower_bound(boost::make_tuple(a.id, witness_id_type()));
        while (itr != vidx.end() && itr->account == a.id) {
            const auto &current = *itr;
            ++itr;
            remove(current);
        }

        if (has_hardfork(STEEMIT_HARDFORK_0_6__104)) { // TODO: this check can be removed after hard fork
            modify(a, [&](account_object &acc) {
                acc.witnesses_voted_for = 0;
            });
        }
    }

    account_name_type get_scheduled_witness(uint32_t slot_num) const {
        const dynamic_global_property_object &dpo = get_dynamic_global_properties();
        const witness_schedule_object &wso = get_witness_schedule_object();
        uint64_t current_aslot = dpo.current_aslot + slot_num;
        return wso.current_shuffled_witnesses[current_aslot %
                                              wso.num_scheduled_witnesses];
    }


    void adjust_witness_votes(const account_object &a, share_type delta) {
        const auto &vidx = get_index<witness_vote_index>().indices().get<by_account_witness>();
        auto itr = vidx.lower_bound(boost::make_tuple(a.id, witness_id_type()));
        while (itr != vidx.end() && itr->account == a.id) {
            adjust_witness_vote(get(itr->witness), delta);
            ++itr;
        }
    }


    void update_median_witness_props() {
        const witness_schedule_object &wso = get_witness_schedule_object();

        /// fetch all witness objects
        vector<const witness_object *> active;
        active.reserve(wso.num_scheduled_witnesses);
        for (int i = 0; i < wso.num_scheduled_witnesses; i++) {
            active.push_back(&get_witness(wso.current_shuffled_witnesses[i]));
        }

        /// sort them by account_creation_fee
        std::sort(active.begin(), active.end(), [&](const witness_object *a, const witness_object *b) {
            return a->props.account_creation_fee.amount <
                   b->props.account_creation_fee.amount;
        });
        asset median_account_creation_fee = active[active.size() /
                                                   2]->props.account_creation_fee;

        /// sort them by maximum_block_size
        std::sort(active.begin(), active.end(), [&](const witness_object *a, const witness_object *b) {
            return a->props.maximum_block_size <
                   b->props.maximum_block_size;
        });
        uint32_t median_maximum_block_size = active[active.size() /
                                                    2]->props.maximum_block_size;

        /// sort them by sbd_interest_rate
        std::sort(active.begin(), active.end(), [&](const witness_object *a, const witness_object *b) {
            return a->props.sbd_interest_rate < b->props.sbd_interest_rate;
        });
        uint16_t median_sbd_interest_rate = active[active.size() /
                                                   2]->props.sbd_interest_rate;

        modify(wso, [&](witness_schedule_object &_wso) {
            _wso.median_props.account_creation_fee = median_account_creation_fee;
            _wso.median_props.maximum_block_size = median_maximum_block_size;
            _wso.median_props.sbd_interest_rate = median_sbd_interest_rate;
        });

        modify(get_dynamic_global_properties(), [&](dynamic_global_property_object &_dgpo) {
            _dgpo.maximum_block_size = median_maximum_block_size;
            _dgpo.sbd_interest_rate = median_sbd_interest_rate;
        });
    }


protected:
    database_basic &references;

};
}}
#endif //GOLOS_WITNESS_POLICY_HPP
