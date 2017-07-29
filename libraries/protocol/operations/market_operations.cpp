#include <steemit/protocol/operations/market_operations.hpp>

#include <fc/utf8.hpp>

namespace steemit {
    namespace protocol {

        /// TODO: after the hardfork, we can rename this method validate_permlink because it is strictily less restrictive than before
        ///  Issue #56 contains the justificiation for allowing any UTF-8 string to serve as a permlink, content will be grouped by tags
        ///  going forward.
        inline void validate_permlink(const string &permlink) {
            FC_ASSERT(permlink.size() <
                      STEEMIT_MAX_PERMLINK_LENGTH, "permlink is too long");
            FC_ASSERT(fc::is_utf8(permlink), "permlink not formatted in UTF8");
        }

        inline void validate_account_name(const string &name) {
            FC_ASSERT(is_valid_account_name(name), "Account name ${n} is invalid", ("n", name));
        }

        bool inline is_asset_type(asset asset, asset_symbol_type symbol) {
            return asset.symbol == symbol;
        }

        void limit_order_create_operation::validate() const {
            validate_account_name(owner);
            (amount_to_sell / min_to_receive).validate();
        }

        void limit_order_create2_operation::validate() const {
            validate_account_name(owner);
            FC_ASSERT(amount_to_sell.symbol ==
                      exchange_rate.base.symbol, "Sell asset must be the base of the price");
            exchange_rate.validate();

            FC_ASSERT((amount_to_sell * exchange_rate).amount >
                      0, "Amount to sell cannot round to 0 when traded");
        }

        void limit_order_cancel_operation::validate() const {
            validate_account_name(owner);
        }

        void convert_operation::validate() const {
            validate_account_name(owner);
            /// only allow conversion from SBD to STEEM, allowing the opposite can enable traders to abuse
            /// market fluxuations through converting large quantities without moving the price.
            FC_ASSERT(is_asset_type(amount, SBD_SYMBOL), "Can only convert SBD to STEEM");
            FC_ASSERT(amount.amount > 0, "Must convert some SBD");
        }

        void call_order_update_operation::validate() const {
            try {
                FC_ASSERT(delta_collateral.symbol != delta_debt.symbol);
                FC_ASSERT(
                        delta_collateral.amount != 0 || delta_debt.amount != 0);
            } FC_CAPTURE_AND_RETHROW((*this))
        }
    }
}