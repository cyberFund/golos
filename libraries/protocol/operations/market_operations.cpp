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

        template<uint8_t m, uint8_t h, uint16_t r>
        void limit_order_create<m, h, r, steemit::type_traits::static_range<(
                h <= 16)>>::validate() const {
            validate_account_name(owner);
            FC_ASSERT((is_asset_type(amount_to_sell, STEEM_SYMBOL) &&
                       is_asset_type(min_to_receive, SBD_SYMBOL))
                      || (is_asset_type(amount_to_sell, SBD_SYMBOL) &&
                          is_asset_type(min_to_receive, STEEM_SYMBOL)),
                    "Limit order must be for the STEEM:SBD market");
            (amount_to_sell / min_to_receive).validate();
        }

        template<uint8_t m = 0, uint8_t h, uint16_t r = 0>
        void limit_order_custom_rate_create<m, h, r, steemit::type_traits::static_range<(
                h <= 16)>>::validate() const {
            validate_account_name(owner);
            FC_ASSERT(amount_to_sell.symbol ==
                      exchange_rate.base.symbol, "Sell asset must be the base of the price");
            exchange_rate.validate();

            FC_ASSERT((is_asset_type(amount_to_sell, STEEM_SYMBOL) &&
                       is_asset_type(exchange_rate.quote, SBD_SYMBOL)) ||
                      (is_asset_type(amount_to_sell, SBD_SYMBOL) &&
                       is_asset_type(exchange_rate.quote, STEEM_SYMBOL)),
                    "Limit order must be for the STEEM:SBD market");

            FC_ASSERT((amount_to_sell * exchange_rate).amount >
                      0, "Amount to sell cannot round to 0 when traded");
        }

        template<uint8_t m, uint8_t h, uint16_t r>
        void limit_order_create<m, h, r, steemit::type_traits::static_range<(
                h == 17)>>::validate() const {
            FC_ASSERT(amount_to_sell.symbol != min_to_receive.symbol);
            FC_ASSERT(fee.amount >= 0);
            FC_ASSERT(amount_to_sell.amount > 0);
            FC_ASSERT(min_to_receive.amount > 0);
        }

        template<uint8_t m, uint8_t h, uint16_t r>
        void limit_order_cancel<m, h, r, steemit::type_traits::static_range<(
                h <= 16)>>::validate() const {
            validate_account_name(owner);
        }

        template<uint8_t m, uint8_t h, uint16_t r>
        void limit_order_cancel<m, h, r, steemit::type_traits::static_range<(
                h == 17)>>::validate() const {
            validate_account_name(owner);
            FC_ASSERT(fee.amount >= 0);
        }

        void convert::validate() const {
            validate_account_name(owner);
            /// only allow conversion from SBD to STEEM, allowing the opposite can enable traders to abuse
            /// market fluxuations through converting large quantities without moving the price.
            FC_ASSERT(is_asset_type(amount, SBD_SYMBOL), "Can only convert SBD to STEEM");
            FC_ASSERT(amount.amount > 0, "Must convert some SBD");
        }
    }
}