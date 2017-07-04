#include <steemit/chain/database.hpp>

#include <steemit/chain/account_object.hpp>

namespace steemit {
    namespace chain {
        share_type cut_fee(share_type a, uint16_t p) {
            if (a == 0 || p == 0) {
                return 0;
            }
            if (p == STEEMIT_100_PERCENT) {
                return a;
            }

            fc::uint128 r(a.value);
            r *= p;
            r /= STEEMIT_100_PERCENT;
            return r.to_uint64();
        }

        void account_balance_object::adjust_balance(const protocol::asset &delta) {
            assert(delta.symbol == asset_type);
            balance += delta.amount;
        }

        void account_statistics_object::process_fees(const account_object &a, database &d) const {
            if (pending_fees > 0 || pending_vested_fees > 0) {
                auto pay_out_fees = [&](const account_object &account, share_type core_fee_total, bool require_vesting) {
                    // Check the referrer -- if he's no longer a member, pay to the lifetime referrer instead.
                    // No need to check the registrar; registrars are required to be lifetime members.
                    if (d.get_account(account.referrer).is_basic_account(d.head_block_time())) {
                        d.modify(account, [](account_object &a) {
                            a.referrer = a.lifetime_referrer;
                        });
                    }

                    share_type network_cut = cut_fee(core_fee_total, account.network_fee_percentage);
                    assert(network_cut <= core_fee_total);

#ifndef NDEBUG
                    const auto &props = d.get_global_properties();

                    share_type reserveed = cut_fee(network_cut, props.parameters.reserve_percent_of_fee);
                    share_type accumulated = network_cut - reserveed;
                    assert(accumulated + reserveed == network_cut);
#endif
                    share_type lifetime_cut = cut_fee(core_fee_total, account.lifetime_referrer_fee_percentage);
                    share_type referral =
                            core_fee_total - network_cut - lifetime_cut;

                    d.modify(d.get_asset_dynamic_data_id_type()(d), [network_cut](asset_dynamic_data_object &d) {
                        d.accumulated_fees += network_cut;
                    });

                    // Potential optimization: Skip some of this math and object lookups by special casing on the account type.
                    // For example, if the account is a lifetime member, we can skip all this and just deposit the referral to
                    // it directly.
                    share_type referrer_cut = cut_fee(referral, account.referrer_rewards_percentage);
                    share_type registrar_cut = referral - referrer_cut;

                    d.deposit_cashback(d.get(account.lifetime_referrer), lifetime_cut, require_vesting);
                    d.deposit_cashback(d.get(account.referrer), referrer_cut, require_vesting);
                    d.deposit_cashback(d.get(account.registrar), registrar_cut, require_vesting);

                    assert(referrer_cut + registrar_cut + accumulated +
                           reserveed + lifetime_cut == core_fee_total);
                };

                pay_out_fees(a, pending_fees, true);
                pay_out_fees(a, pending_vested_fees, false);

                d.modify(*this, [&](account_statistics_object &s) {
                    s.lifetime_fees_paid += pending_fees + pending_vested_fees;
                    s.pending_fees = 0;
                    s.pending_vested_fees = 0;
                });
            }
        }

        void account_statistics_object::pay_fee(share_type core_fee, share_type cashback_vesting_threshold) {
            if (core_fee > cashback_vesting_threshold) {
                pending_fees += core_fee;
            } else {
                pending_vested_fees += core_fee;
            }
        }
    }
}