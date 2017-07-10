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

        void account_statistics_object::pay_fee(share_type core_fee, share_type cashback_vesting_threshold) {
            if (core_fee > cashback_vesting_threshold) {
                pending_fees += core_fee;
            } else {
                pending_vested_fees += core_fee;
            }
        }
    }
}