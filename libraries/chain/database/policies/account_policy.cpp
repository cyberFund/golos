#include <steemit/chain/database/policies/account_policy.hpp>
#include <steemit/chain/database/database_basic.hpp>
#include <steemit/chain/compound.hpp>

namespace steemit {
    namespace chain {
        namespace {
            asset get_content_reward(database_basic &db) {
                const auto &props = db.get_dynamic_global_properties();
                auto reward = asset(255, STEEM_SYMBOL);
                static_assert(STEEMIT_BLOCK_INTERVAL == 3, "this code assumes a 3-second time interval");
                if (props.head_block_number > STEEMIT_START_VESTING_BLOCK) {
                    asset percent(protocol::calc_percent_reward_per_block<STEEMIT_CONTENT_APR_PERCENT>(
                            props.virtual_supply.amount), STEEM_SYMBOL);
                    reward = std::max(percent, STEEMIT_MIN_CONTENT_REWARD);
                }

                return reward;
            }


            asset get_curation_reward(database_basic &db) {
                const auto &props = db.get_dynamic_global_properties();
                auto reward = asset(85, STEEM_SYMBOL);
                static_assert(STEEMIT_BLOCK_INTERVAL == 3, "this code assumes a 3-second time interval");
                if (props.head_block_number > STEEMIT_START_VESTING_BLOCK) {
                    asset percent(protocol::calc_percent_reward_per_block<STEEMIT_CURATE_APR_PERCENT>(
                            props.virtual_supply.amount), STEEM_SYMBOL);
                    reward = std::max(percent, STEEMIT_MIN_CURATE_REWARD);
                }

                return reward;
            }

            const witness_schedule_object &get_witness_schedule_object(database_basic &db) {
                try {
                    return db.get<witness_schedule_object>();
                } FC_CAPTURE_AND_RETHROW()
            }

            const witness_object &get_witness(database_basic &db, const account_name_type &name) {
                try {
                    return db.get<witness_object, by_name>(name);
                } FC_CAPTURE_AND_RETHROW((name))
            }
        }

        const account_object &account_policy::get_account(const account_name_type &name) const {
            try {
                return references.get<account_object, by_name>(name);
            } FC_CAPTURE_AND_RETHROW((name))
        }

        const account_object *account_policy::find_account(const account_name_type &name) const {
            return references.find<account_object, by_name>(name);
        }

        asset account_policy::get_balance(const account_object &a, asset_symbol_type symbol) const {
            switch (symbol) {
                case STEEM_SYMBOL:
                    return a.balance;
                case SBD_SYMBOL:
                    return a.sbd_balance;
                default:
                    FC_ASSERT(false, "invalid symbol");
            }
        }

        asset account_policy::get_savings_balance(const account_object &a, asset_symbol_type symbol) const {
            switch (symbol) {
                case STEEM_SYMBOL:
                    return a.savings_balance;
                case SBD_SYMBOL:
                    return a.savings_sbd_balance;
                default:
                    FC_ASSERT(!"invalid symbol");
            }
        }

        void account_policy::adjust_savings_balance(const account_object &a, const asset &delta) {
            references.modify(a, [&](account_object &acnt) {
                switch (delta.symbol) {
                    case STEEM_SYMBOL:
                        acnt.savings_balance += delta;
                        break;
                    case SBD_SYMBOL:
                        if (a.savings_sbd_seconds_last_update != references.head_block_time()) {
                            acnt.savings_sbd_seconds += fc::uint128_t(a.savings_sbd_balance.amount.value) *
                                                        (references.head_block_time() -
                                                         a.savings_sbd_seconds_last_update).to_seconds();
                            acnt.savings_sbd_seconds_last_update = references.head_block_time();

                            if (acnt.savings_sbd_seconds > 0 && (acnt.savings_sbd_seconds_last_update -
                                                                 acnt.savings_sbd_last_interest_payment).to_seconds() >
                                                                STEEMIT_SBD_INTEREST_COMPOUND_INTERVAL_SEC) {
                                auto interest = acnt.savings_sbd_seconds / STEEMIT_SECONDS_PER_YEAR;
                                interest *= references.get_dynamic_global_properties().sbd_interest_rate;
                                interest /= STEEMIT_100_PERCENT;
                                asset interest_paid(interest.to_uint64(), SBD_SYMBOL);
                                acnt.savings_sbd_balance += interest_paid;
                                acnt.savings_sbd_seconds = 0;
                                acnt.savings_sbd_last_interest_payment = references.head_block_time();

                                references.push_virtual_operation(interest_operation(a.name, interest_paid));

                                references.modify(references.get_dynamic_global_properties(),
                                                  [&](dynamic_global_property_object &props) {
                                                      props.current_sbd_supply += interest_paid;
                                                      props.virtual_supply += interest_paid *
                                                                              references.get_feed_history().current_median_history;
                                                  });
                            }
                        }
                        acnt.savings_sbd_balance += delta;
                        break;
                    default:
                        FC_ASSERT(!"invalid symbol");
                }
            });
        }

        void account_policy::adjust_balance(const account_object &a, const asset &delta) {
            references.dynamic_extension_worker().get("account")->invoke("adjust_balance", a, delta);
        }

        void account_policy::update_owner_authority(const account_object &account, const authority &owner_authority) {
            if (references.head_block_num() >= STEEMIT_OWNER_AUTH_HISTORY_TRACKING_START_BLOCK_NUM) {
                references.create<owner_authority_history_object>([&](owner_authority_history_object &hist) {
                    hist.account = account.name;
                    hist.previous_owner_authority = references.get<account_authority_object, by_account>(
                            account.name).owner;
                    hist.last_valid_time = references.head_block_time();
                });
            }

            references.modify(references.get<account_authority_object, by_account>(account.name),
                              [&](account_authority_object &auth) {
                                  auth.owner = owner_authority;
                                  auth.last_owner_update = references.head_block_time();
                              });
        }

        void account_policy::clear_null_account_balance() {
            if (!references.has_hardfork(STEEMIT_HARDFORK_0_14__327)) {
                return;
            }

            const auto &null_account = get_account(STEEMIT_NULL_ACCOUNT);
            asset total_steem(0, STEEM_SYMBOL);
            asset total_sbd(0, SBD_SYMBOL);

            if (null_account.balance.amount > 0) {
                total_steem += null_account.balance;
                adjust_balance(null_account, -null_account.balance);
            }

            if (null_account.savings_balance.amount > 0) {
                total_steem += null_account.savings_balance;
                adjust_savings_balance(null_account, -null_account.savings_balance);
            }

            if (null_account.sbd_balance.amount > 0) {
                total_sbd += null_account.sbd_balance;
                adjust_balance(null_account, -null_account.sbd_balance);
            }

            if (null_account.savings_sbd_balance.amount > 0) {
                total_sbd += null_account.savings_sbd_balance;
                adjust_savings_balance(null_account, -null_account.savings_sbd_balance);
            }

            if (null_account.vesting_shares.amount > 0) {
                const auto &gpo = references.get_dynamic_global_properties();
                auto converted_steem = null_account.vesting_shares * gpo.get_vesting_share_price();

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
                references.dynamic_extension_worker().get("asset")->invoke("adjust_supply", -total_steem);
            }

            if (total_sbd.amount > 0) {
                references.dynamic_extension_worker().get("asset")->invoke("adjust_supply", -total_sbd);
            }
        }

        void account_policy::pay_fee(const account_object &account, asset fee) {
            FC_ASSERT(fee.amount >= 0); /// NOTE if this fails then validate() on some operation is probably wrong
            if (fee.amount == 0) {
                return;
            }

            FC_ASSERT(account.balance >= fee);
            adjust_balance(account, -fee);
            references.dynamic_extension_worker().get("asset")->invoke("adjust_supply", -fee);
        }

        void account_policy::old_update_account_bandwidth(const account_object &a, uint32_t trx_size,
                                                          const bandwidth_type type) {
            try {
                const auto &props = references.get_dynamic_global_properties();
                if (props.total_vesting_shares.amount > 0) {
                    FC_ASSERT(a.vesting_shares.amount > 0,
                              "Only accounts with a postive vesting balance may transact.");

                    auto band = references.find<account_bandwidth_object, by_account_bandwidth_type>(
                            boost::make_tuple(a.name, type));

                    if (band == nullptr) {
                        band = &references.create<account_bandwidth_object>([&](account_bandwidth_object &b) {
                            b.account = a.name;
                            b.type = type;
                        });
                    }

                    references.modify(*band, [&](account_bandwidth_object &b) {
                        b.lifetime_bandwidth += trx_size * STEEMIT_BANDWIDTH_PRECISION;

                        auto now = references.head_block_time();
                        auto delta_time = (now - b.last_bandwidth_update).to_seconds();
                        uint64_t N = trx_size * STEEMIT_BANDWIDTH_PRECISION;
                        if (delta_time >= STEEMIT_BANDWIDTH_AVERAGE_WINDOW_SECONDS) {
                            b.average_bandwidth = N;
                        } else {
                            auto old_weight =
                                    b.average_bandwidth * (STEEMIT_BANDWIDTH_AVERAGE_WINDOW_SECONDS - delta_time);
                            auto new_weight = delta_time * N;
                            b.average_bandwidth = (old_weight + new_weight) / STEEMIT_BANDWIDTH_AVERAGE_WINDOW_SECONDS;
                        }

                        b.last_bandwidth_update = now;
                    });

                    fc::uint128 account_vshares(a.effective_vesting_shares().amount.value);
                    fc::uint128 total_vshares(props.total_vesting_shares.amount.value);

                    fc::uint128 account_average_bandwidth(band->average_bandwidth.value);
                    fc::uint128 max_virtual_bandwidth(props.max_virtual_bandwidth);

                    FC_ASSERT((account_vshares * max_virtual_bandwidth) > (account_average_bandwidth * total_vshares),
                              "Account exceeded maximum allowed bandwidth per vesting share.",
                              ("account_vshares", account_vshares)("account_average_bandwidth",
                                                                   account_average_bandwidth)("max_virtual_bandwidth",
                                                                                              max_virtual_bandwidth)(
                                      "total_vesting_shares", total_vshares));
                }
            } FC_CAPTURE_AND_RETHROW()
        }

        /*
                bool account_policy::update_account_bandwidth(const account_object &a, uint32_t trx_size,
                                                         const bandwidth_type type) {
                    const auto &props = references.get_dynamic_global_properties();
                    bool has_bandwidth = true;

                    if (props.total_vesting_shares.amount > 0) {
                        auto band = references.find<account_bandwidth_object, by_account_bandwidth_type>(
                                boost::make_tuple(a.name, type));

                        if (band == nullptr) {
                            band = &references.create<account_bandwidth_object>([&](account_bandwidth_object &b) {
                                b.account = a.name;
                                b.type = type;
                            });
                        }

                        share_type new_bandwidth;
                        share_type trx_bandwidth = trx_size * STEEMIT_BANDWIDTH_PRECISION;
                        auto delta_time = (references.head_block_time() - band->last_bandwidth_update).to_seconds();

                        if (delta_time > STEEMIT_BANDWIDTH_AVERAGE_WINDOW_SECONDS) {
                            new_bandwidth = 0;
                        } else {
                            new_bandwidth = (((STEEMIT_BANDWIDTH_AVERAGE_WINDOW_SECONDS - delta_time) * fc::uint128(band->average_bandwidth.value)) / STEEMIT_BANDWIDTH_AVERAGE_WINDOW_SECONDS).to_uint64();
                        }

                        new_bandwidth += trx_bandwidth;

                        references.modify(*band, [&](account_bandwidth_object &b) {
                            b.average_bandwidth = new_bandwidth;
                            b.lifetime_bandwidth += trx_bandwidth;
                            b.last_bandwidth_update = references.head_block_time();
                        });

                        fc::uint128 account_vshares(a.vesting_shares.amount.value);
                        fc::uint128 total_vshares(props.total_vesting_shares.amount.value);
                        fc::uint128 account_average_bandwidth(band->average_bandwidth.value);
                        fc::uint128 max_virtual_bandwidth(props.max_virtual_bandwidth);

                        has_bandwidth = (account_vshares * max_virtual_bandwidth) >
                                        (account_average_bandwidth * total_vshares);

                        if (references.is_producing())
                            FC_ASSERT(has_bandwidth,
                                      "Account exceeded maximum allowed bandwidth per vesting share.",
                                      ("account_vshares", account_vshares)
                                              ("account_average_bandwidth", account_average_bandwidth)
                                              ("max_virtual_bandwidth", max_virtual_bandwidth)
                                              ("total_vesting_shares", total_vshares));
                    }

                    return has_bandwidth;
                }
        */
        void account_policy::expire_escrow_ratification() {
            const auto &escrow_idx = references.get_index<escrow_index>().indices().get<by_ratification_deadline>();
            auto escrow_itr = escrow_idx.lower_bound(false);

            while (escrow_itr != escrow_idx.end() && !escrow_itr->is_approved() &&
                   escrow_itr->ratification_deadline <= references.head_block_time()) {
                const auto &old_escrow = *escrow_itr;
                ++escrow_itr;

                const auto &from_account = get_account(old_escrow.from);
                adjust_balance(from_account, old_escrow.steem_balance);
                adjust_balance(from_account, old_escrow.sbd_balance);
                adjust_balance(from_account, old_escrow.pending_fee);
                references.remove(old_escrow);
            }
        }

        asset account_policy::get_balance(const string &aname, asset_symbol_type symbol) const {
            return get_balance(get_account(aname), symbol);
        }

        void account_policy::process_funds() {
            const auto &props = references.get_dynamic_global_properties();
            const auto &wso = get_witness_schedule_object(references);

            if (references.has_hardfork(STEEMIT_HARDFORK_0_16__551)) {
                /**
       * At block 7,000,000 have a 9.5% instantaneous inflation rate, decreasing to 0.95% at a rate of 0.01%
       * every 250k blocks. This narrowing will take approximately 20.5 years and will complete on block 220,750,000
       */
                int64_t start_inflation_rate = int64_t(STEEMIT_INFLATION_RATE_START_PERCENT);
                int64_t inflation_rate_adjustment = int64_t(
                        references.head_block_num() / STEEMIT_INFLATION_NARROWING_PERIOD);
                int64_t inflation_rate_floor = int64_t(STEEMIT_INFLATION_RATE_STOP_PERCENT);

                // below subtraction cannot underflow int64_t because inflation_rate_adjustment is <2^32
                int64_t current_inflation_rate = std::max(start_inflation_rate - inflation_rate_adjustment,
                                                          inflation_rate_floor);

                auto new_steem = (props.virtual_supply.amount * current_inflation_rate) /
                                 (int64_t(STEEMIT_100_PERCENT) * int64_t(STEEMIT_BLOCKS_PER_YEAR));
                auto content_reward = (new_steem * STEEMIT_CONTENT_REWARD_PERCENT) / STEEMIT_100_PERCENT;
                if (references.has_hardfork(STEEMIT_HARDFORK_0_17__86)) {
                    content_reward = dynamic_extension::cast<share_type>(
                            references.dynamic_extension_worker().get("behaviour_based")->invoke("pay_reward_funds",
                                                                                                 content_reward));
                } /// 75% to content creator
                auto vesting_reward =
                        (new_steem * STEEMIT_VESTING_FUND_PERCENT) / STEEMIT_100_PERCENT; /// 15% to vesting fund
                auto witness_reward = new_steem - content_reward - vesting_reward; /// Remaining 10% to witness pay

                const auto &cwit = get_witness(references, props.current_witness);
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
                    if (!references.has_hardfork(STEEMIT_HARDFORK_0_17__86)) {
                        p.total_reward_fund_steem += asset(content_reward, STEEM_SYMBOL);
                    }
                    p.current_supply += asset(new_steem, STEEM_SYMBOL);
                    p.virtual_supply += asset(new_steem, STEEM_SYMBOL);

                });

                references.dynamic_extension_worker().get("witness")->invoke("create_vesting", get_account(cwit.owner),
                                                                             asset(witness_reward, STEEM_SYMBOL));
            } else {
                auto content_reward = get_content_reward(references);
                auto curate_reward = get_curation_reward(references);
                auto witness_pay = get_producer_reward();
                auto vesting_reward = content_reward + curate_reward + witness_pay;

                content_reward = content_reward + curate_reward;

                if (props.head_block_number < STEEMIT_START_VESTING_BLOCK) {
                    vesting_reward.amount = 0;
                } else {
                    vesting_reward.amount.value *= 9;
                }

                references.modify(props, [&](dynamic_global_property_object &p) {
                    p.total_vesting_fund_steem += vesting_reward;
                    p.total_reward_fund_steem += content_reward;
                    p.current_supply += content_reward + witness_pay + vesting_reward;
                    p.virtual_supply += content_reward + witness_pay + vesting_reward;
                });
            }
        }

        void account_policy::process_savings_withdraws() {
            const auto &idx = references.get_index<savings_withdraw_index>().indices().get<by_complete_from_rid>();
            auto itr = idx.begin();
            while (itr != idx.end()) {

                if (itr->complete > references.head_block_time()) {
                    break;
                }

                adjust_balance(get_account(itr->to), itr->amount);

                references.modify(get_account(itr->from), [&](account_object &a) {
                    a.savings_withdraw_requests--;
                });

                references.push_virtual_operation(
                        fill_transfer_from_savings_operation(itr->from, itr->to, itr->amount, itr->request_id,
                                                             to_string(itr->memo)));

                references.remove(*itr);
                itr = idx.begin();
            }
        }

        asset account_policy::get_producer_reward() {
            const auto &props = references.get_dynamic_global_properties();
            static_assert(STEEMIT_BLOCK_INTERVAL == 3, "this code assumes a 3-second time interval");
            asset percent(
                    protocol::calc_percent_reward_per_block<STEEMIT_PRODUCER_APR_PERCENT>(props.virtual_supply.amount),
                    STEEM_SYMBOL);

            const auto &witness_account = get_account(props.current_witness);

            if (references.has_hardfork(STEEMIT_HARDFORK_0_16)) {
                auto pay = std::max(percent, STEEMIT_MIN_PRODUCER_REWARD);

                /// pay witness in vesting shares
                if (props.head_block_number >= STEEMIT_START_MINER_VOTING_BLOCK ||
                    (witness_account.vesting_shares.amount.value == 0)) {
                    // const auto& witness_obj = get_witness( props.current_witness );
                    references.dynamic_extension_worker().get("witness")->invoke("create_vesting", witness_account,
                                                                                 pay);
                } else {
                    references.modify(get_account(witness_account.name), [&](account_object &a) {
                        a.balance += pay;
                    });
                }

                return pay;
            } else {
                auto pay = std::max(percent, STEEMIT_MIN_PRODUCER_REWARD_PRE_HF16);

                /// pay witness in vesting shares
                if (props.head_block_number >= STEEMIT_START_MINER_VOTING_BLOCK ||
                    (witness_account.vesting_shares.amount.value == 0)) {
                    // const auto& witness_obj = get_witness( props.current_witness );
                    references.dynamic_extension_worker().get("witness")->invoke("create_vesting", witness_account,
                                                                                 pay);
                } else {
                    references.modify(get_account(witness_account.name), [&](account_object &a) {
                        a.balance += pay;
                    });
                }

                return pay;
            }
        }

        void account_policy::account_recovery_processing() {
            // Clear expired recovery requests
            const auto &rec_req_idx = references.get_index<account_recovery_request_index>().indices().get<
                    by_expiration>();
            auto rec_req = rec_req_idx.begin();

            while (rec_req != rec_req_idx.end() && rec_req->expires <= references.head_block_time()) {
                references.remove(*rec_req);
                rec_req = rec_req_idx.begin();
            }

            // Clear invalid historical authorities
            const auto &hist_idx = references.get_index<owner_authority_history_index>().indices(); //by id
            auto hist = hist_idx.begin();

            while (hist != hist_idx.end() &&
                   time_point_sec(hist->last_valid_time + STEEMIT_OWNER_AUTH_RECOVERY_PERIOD) <
                   references.head_block_time()) {
                references.remove(*hist);
                hist = hist_idx.begin();
            }

            // Apply effective recovery_account changes
            const auto &change_req_idx = references.get_index<change_recovery_account_request_index>().indices().get<
                    by_effective_date>();
            auto change_req = change_req_idx.begin();

            while (change_req != change_req_idx.end() && change_req->effective_on <= references.head_block_time()) {
                references.modify(get_account(change_req->account_to_recover), [&](account_object &a) {
                    a.recovery_account = change_req->recovery_account;
                });

                references.remove(*change_req);
                change_req = change_req_idx.begin();
            }
        }

        void account_policy::clear_expired_delegations() {
            auto now = references.head_block_time();
            const auto &delegations_by_exp = references.get_index<vesting_delegation_expiration_index, by_expiration>();
            auto itr = delegations_by_exp.begin();
            while (itr != delegations_by_exp.end() && itr->expiration < now) {
                references.modify(get_account(itr->delegator), [&](account_object &a) {
                    a.delegated_vesting_shares -= itr->vesting_shares;
                });

                references.push_virtual_operation(
                        return_vesting_delegation_operation(itr->delegator, itr->vesting_shares));

                references.remove(*itr);
                itr = delegations_by_exp.begin();
            }
        }

        account_policy::account_policy(database_basic &ref, int) : generic_policy(ref) {
        }

    }
}