#include <steemit/protocol/market_operations.hpp>

namespace steemit {
    namespace protocol {

        void limit_order_create_operation::validate() const {
            FC_ASSERT(amount_to_sell.symbol != min_to_receive.symbol);
            FC_ASSERT(fee.amount >= 0);
            FC_ASSERT(amount_to_sell.amount > 0);
            FC_ASSERT(min_to_receive.amount > 0);
        }

        void limit_order_cancel_operation::validate() const {
            FC_ASSERT(fee.amount >= 0);
        }

        void call_order_update_operation::validate() const {
            try {
                FC_ASSERT(fee.amount >= 0);
                FC_ASSERT(delta_collateral.symbol != delta_debt.symbol);
                FC_ASSERT(delta_collateral.amount != 0 || delta_debt.amount != 0);
            }
            FC_CAPTURE_AND_RETHROW((*this))
        }
    }
}
