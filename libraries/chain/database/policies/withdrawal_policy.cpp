#include <steemit/chain/database/policies/withdrawal_policy.hpp>
#include <steemit/chain/database/database_basic.hpp>
#include <steemit/chain/chain_objects/steem_objects.hpp>

namespace steemit {
    namespace chain {

        withdrawal_policy::withdrawal_policy(database_basic &ref, int) : generic_policy(ref) {

        }

        void withdrawal_policy::process_vesting_withdrawals() {
            const auto &widx = references.get_index<account_index>().indices().get<by_next_vesting_withdrawal>();
            const auto &didx = references.get_index<withdraw_vesting_route_index>().indices().get<by_withdraw_route>();
            auto current = widx.begin();

            const auto &cprops = references.get_dynamic_global_properties();

            while (current != widx.end() && current->next_vesting_withdrawal <= references.head_block_time()) {
                const auto &from_account = *current;
                ++current;

                /**
      *  Let T = total tokens in vesting fund
      *  Let V = total vesting shares
      *  Let v = total vesting shares being cashed out
      *
      *  The user may withdraw  vT / V tokens
      */
                share_type to_withdraw;
                if (from_account.to_withdraw - from_account.withdrawn < from_account.vesting_withdraw_rate.amount) {
                    to_withdraw = std::min(from_account.vesting_shares.amount,
                                           from_account.to_withdraw % from_account.vesting_withdraw_rate.amount).value;
                } else {
                    to_withdraw = std::min(from_account.vesting_shares.amount,
                                           from_account.vesting_withdraw_rate.amount).value;
                }

                share_type vests_deposited_as_steem = 0;
                share_type vests_deposited_as_vests = 0;
                asset total_steem_converted = asset(0, STEEM_SYMBOL);

                // Do two passes, the first for vests, the second for steem. Try to maintain as much accuracy for vests as possible.
                for (auto itr = didx.upper_bound(boost::make_tuple(from_account.id, account_id_type()));
                     itr != didx.end() && itr->from_account == from_account.id; ++itr) {
                    if (itr->auto_vest) {
                        share_type to_deposit = ((fc::uint128_t(to_withdraw.value) * itr->percent) /
                                                 STEEMIT_100_PERCENT).to_uint64();
                        vests_deposited_as_vests += to_deposit;

                        if (to_deposit > 0) {
                            const auto &to_account = references.get(itr->to_account);

                            references.modify(to_account, [&](account_object &a) {
                                a.vesting_shares.amount += to_deposit;
                            });

                            references.dynamic_extension_worker().get("witness")->invoke("adjust_proxied_witness_votes",
                                                                                         to_account, to_deposit);

                            references.push_virtual_operation(
                                    fill_vesting_withdraw_operation(from_account.name, to_account.name,
                                                                    asset(to_deposit, VESTS_SYMBOL),
                                                                    asset(to_deposit, VESTS_SYMBOL)));
                        }
                    }
                }

                for (auto itr = didx.upper_bound(boost::make_tuple(from_account.id, account_id_type()));
                     itr != didx.end() && itr->from_account == from_account.id; ++itr) {
                    if (!itr->auto_vest) {
                        const auto &to_account = references.get(itr->to_account);

                        share_type to_deposit = ((fc::uint128_t(to_withdraw.value) * itr->percent) /
                                                 STEEMIT_100_PERCENT).to_uint64();
                        vests_deposited_as_steem += to_deposit;
                        auto converted_steem = asset(to_deposit, VESTS_SYMBOL) * cprops.get_vesting_share_price();
                        total_steem_converted += converted_steem;

                        if (to_deposit > 0) {
                            references.modify(to_account, [&](account_object &a) {
                                a.balance += converted_steem;
                            });

                            references.modify(cprops, [&](dynamic_global_property_object &o) {
                                o.total_vesting_fund_steem -= converted_steem;
                                o.total_vesting_shares.amount -= to_deposit;
                            });

                            references.push_virtual_operation(
                                    fill_vesting_withdraw_operation(from_account.name, to_account.name,
                                                                    asset(to_deposit, VESTS_SYMBOL), converted_steem));
                        }
                    }
                }

                share_type to_convert = to_withdraw - vests_deposited_as_steem - vests_deposited_as_vests;
                FC_ASSERT(to_convert >= 0, "Deposited more vests than were supposed to be withdrawn");

                auto converted_steem = asset(to_convert, VESTS_SYMBOL) * cprops.get_vesting_share_price();

                references.modify(from_account, [&](account_object &a) {
                    a.vesting_shares.amount -= to_withdraw;
                    a.balance += converted_steem;
                    a.withdrawn += to_withdraw;

                    if (a.withdrawn >= a.to_withdraw || a.vesting_shares.amount == 0) {
                        a.vesting_withdraw_rate.amount = 0;
                        a.next_vesting_withdrawal = fc::time_point_sec::maximum();
                    } else {
                        a.next_vesting_withdrawal += fc::seconds(STEEMIT_VESTING_WITHDRAW_INTERVAL_SECONDS);
                    }
                });

                references.modify(cprops, [&](dynamic_global_property_object &o) {
                    o.total_vesting_fund_steem -= converted_steem;
                    o.total_vesting_shares.amount -= to_convert;
                });

                if (to_withdraw > 0) {
                    references.dynamic_extension_worker().get("witness")->invoke("adjust_proxied_witness_votes",
                                                                                 from_account, -to_withdraw);
                }

                references.push_virtual_operation(fill_vesting_withdraw_operation(from_account.name, from_account.name,
                                                                                  asset(to_withdraw, VESTS_SYMBOL),
                                                                                  converted_steem));
            }
        }

        const savings_withdraw_object &withdrawal_policy::get_savings_withdraw(const account_name_type &owner,
                                                                               uint32_t request_id) const {
            try {
                return references.get<savings_withdraw_object, by_from_rid>(boost::make_tuple(owner, request_id));
            } FC_CAPTURE_AND_RETHROW((owner)(request_id))
        }

        const savings_withdraw_object *withdrawal_policy::find_savings_withdraw(const account_name_type &owner,
                                                                                uint32_t request_id) const {
            return references.find<savings_withdraw_object, by_from_rid>(boost::make_tuple(owner, request_id));
        }
    }
}