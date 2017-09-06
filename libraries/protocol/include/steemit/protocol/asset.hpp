#pragma once

#include <steemit/protocol/types.hpp>
#include <steemit/protocol/config.hpp>

namespace steemit {
    namespace protocol {

        typedef uint64_t asset_symbol_type;

        extern const int64_t scaled_precision_lut[];

        struct asset {
            asset();

            asset(share_type a, asset_symbol_type name);

            asset(share_type a, asset_name_type name = STEEM_SYMBOL_NAME, uint8_t d = 3);

            share_type amount;
            asset_name_type symbol;
            uint8_t decimals;

            double to_real() const {
                return double(amount.value) / precision();
            }

            asset_symbol_type symbol_type_value() const;

            int64_t precision() const;

            static asset from_string(const string &from);

            string to_string() const;

            asset &operator+=(const asset &o) {
                FC_ASSERT(symbol == o.symbol);
                amount += o.amount;
                return *this;
            }

            asset &operator-=(const asset &o) {
                FC_ASSERT(symbol == o.symbol);
                amount -= o.amount;
                return *this;
            }

            asset operator-() const {
                return asset(-amount, symbol);
            }

            friend bool operator==(const asset &a, const asset &b) {
                return std::tie(a.symbol, a.amount) == std::tie(b.symbol, b.amount);
            }

            friend bool operator<(const asset &a, const asset &b) {
                FC_ASSERT(a.symbol == b.symbol);
                return std::tie(a.amount, a.symbol) < std::tie(b.amount, b.symbol);
            }

            friend bool operator<=(const asset &a, const asset &b) {
                return (a == b) || (a < b);
            }

            friend bool operator!=(const asset &a, const asset &b) {
                return !(a == b);
            }

            friend bool operator>(const asset &a, const asset &b) {
                return !(a <= b);
            }

            friend bool operator>=(const asset &a, const asset &b) {
                return !(a < b);
            }

            friend asset operator-(const asset &a, const asset &b) {
                FC_ASSERT(a.symbol == b.symbol);
                return asset(a.amount - b.amount, a.symbol);
            }

            friend asset operator+(const asset &a, const asset &b) {
                FC_ASSERT(a.symbol == b.symbol);
                return asset(a.amount + b.amount, a.symbol);
            }

            static share_type scaled_precision(uint8_t precision) {
                FC_ASSERT(precision < 19);
                return scaled_precision_lut[precision];
            }
        };

        struct price {
            price(const asset &base = asset(0, STEEM_SYMBOL_NAME), const asset &quote = asset(0, STEEM_SYMBOL_NAME)) : base(base),
                                                                                                             quote(quote) {
            }

            asset base;
            asset quote;

            static price max(asset_name_type base, asset_name_type quote);

            static price min(asset_name_type base, asset_name_type quote);

            price max() const {
                return price::max(base.symbol, quote.symbol);
            }

            price min() const {
                return price::min(base.symbol, quote.symbol);
            }

            double to_real() const {
                return base.to_real() / quote.to_real();
            }

            bool is_null() const;

            void validate() const;

            /**
             *  The black swan price is defined as debt/collateral, we want to perform a margin call
             *  before debt == collateral. Given a debt/collateral ratio of 1 USD / CORE and
             *  a maintenance collateral requirement of 2x we can define the call price to be
             *  2 USD / CORE.
             *
             *  This method divides the collateral by the maintenance collateral ratio to derive
             *  a call price for the given black swan ratio.
             *
             *  There exists some cases where the debt and collateral values are so small that
             *  dividing by the collateral ratio will result in a 0 price or really poor
             *  rounding errors.   No matter what the collateral part of the price ratio can
             *  never go to 0 and the debt can never go more than STEEMIT_MAX_SHARE_SUPPLY
             *
             *  CR * DEBT/COLLAT or DEBT/(COLLAT/CR)
             *
             * @param debt
             * @param collateral
             * @param collateral_ratio
             * @return
             */

            static price call_price(const asset &debt, const asset &collateral, uint16_t collateral_ratio);

            /// The unit price for an asset type A is defined to be a price such that for any asset m, m*A=m
            static price unit_price(asset_name_type a = STEEM_SYMBOL_NAME) {
                return price(asset(1, a), asset(1, a));
            }
        };

        price operator/(const asset &base, const asset &quote);

        inline price operator~(const price &p) {
            return price{p.quote, p.base};
        }

        bool operator<(const asset &a, const asset &b);

        bool operator<=(const asset &a, const asset &b);

        bool operator<(const price &a, const price &b);

        bool operator<=(const price &a, const price &b);

        bool operator>(const price &a, const price &b);

        bool operator>=(const price &a, const price &b);

        bool operator==(const price &a, const price &b);

        bool operator!=(const price &a, const price &b);

        asset operator*(const asset &a, const price &b);

        /**
         *  @class price_feed
         *  @brief defines market parameters for margin positions
         */
        struct price_feed {
            /**
             *  Required maintenance collateral is defined
             *  as a fixed point number with a maximum value of 10.000
             *  and a minimum value of 1.000.  (denominated in GRAPHENE_COLLATERAL_RATIO_DENOM)
             *
             *  A black swan event occurs when value_of_collateral equals
             *  value_of_debt, to avoid a black swan a margin call is
             *  executed when value_of_debt * required_maintenance_collateral
             *  equals value_of_collateral using rate.
             *
             *  Default requirement is $1.75 of collateral per $1 of debt
             *
             *  BlackSwan ---> SQR ---> MCR ----> SP
             */
            ///@{
            /**
             * Forced settlements will evaluate using this price, defined as BITASSET / COLLATERAL
             */
            price settlement_price;

            /// Price at which automatically exchanging this asset for CORE from fee pool occurs (used for paying fees)
            price core_exchange_rate;

            /** Fixed point between 1.000 and 10.000, implied fixed point denominator is GRAPHENE_COLLATERAL_RATIO_DENOM */
            uint16_t maintenance_collateral_ratio = GRAPHENE_DEFAULT_MAINTENANCE_COLLATERAL_RATIO;

            /** Fixed point between 1.000 and 10.000, implied fixed point denominator is GRAPHENE_COLLATERAL_RATIO_DENOM */
            uint16_t maximum_short_squeeze_ratio = GRAPHENE_DEFAULT_MAX_SHORT_SQUEEZE_RATIO;

            /**
             *  When updating a call order the following condition must be maintained:
             *
             *  debt * maintenance_price() < collateral
             *  debt * settlement_price    < debt * maintenance
             *  debt * maintenance_price() < debt * max_short_squeeze_price()
            price maintenance_price()const;
             */

            /** When selling collateral to pay off debt, the least amount of debt to receive should be
             *  min_usd = max_short_squeeze_price() * collateral
             *
             *  This is provided to ensure that a black swan cannot be trigged due to poor liquidity alone, it
             *  must be confirmed by having the max_short_squeeze_price() move below the black swan price.
             */
            price max_short_squeeze_price() const;
            ///@}

            friend bool operator==(const price_feed &a, const price_feed &b) {
                return std::tie(a.settlement_price, a.maintenance_collateral_ratio, a.maximum_short_squeeze_ratio) ==
                       std::tie(b.settlement_price, b.maintenance_collateral_ratio, b.maximum_short_squeeze_ratio);
            }

            void validate() const;

            bool is_for(asset_name_type asset_name) const;
        };
    }
} // steemit::protocol

namespace fc {
    inline void to_variant(const steemit::protocol::asset &var, fc::variant &vo) {
        vo = var.to_string();
    }

    inline void from_variant(const fc::variant &var, steemit::protocol::asset &vo) {
        vo = steemit::protocol::asset::from_string(var.as_string());
    }
}

FC_REFLECT(steemit::protocol::asset, (amount)(symbol)(decimals))
FC_REFLECT(steemit::protocol::price, (base)(quote))

FC_REFLECT(steemit::protocol::price_feed, (settlement_price)(maintenance_collateral_ratio)(maximum_short_squeeze_ratio)(core_exchange_rate))