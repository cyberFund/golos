#pragma once

#include <steemit/protocol/asset.hpp>
#include <steemit/protocol/types.hpp>

namespace steemit {
    namespace chain {

        /**
         *  @brief an offer to sell a amount of a asset at a specified exchange rate by a certain time
         *  @ingroup object
         *  @ingroup protocol
         *  @ingroup market
         *
         *  This limit_order_objects are indexed by @ref expiration and is automatically deleted on the first block after expiration.
         */
        class limit_order_object
                : public object<limit_order_object_type, limit_order_object> {
        public:
            template<typename Constructor, typename Allocator>
            limit_order_object(Constructor &&c, allocator <Allocator> a) {
                c(*this);
            }

            limit_order_object() {

            }

            id_type id;

            time_point_sec created;
            time_point_sec expiration;
            account_name_type seller;
            uint32_t orderid = 0;
            share_type for_sale; ///< asset id is sell_price.base.symbol
            price sell_price;

            pair <asset_symbol_type, asset_symbol_type> get_market() const {
                return sell_price.base.symbol < sell_price.quote.symbol ?
                       std::make_pair(sell_price.base.symbol, sell_price.quote.symbol)
                                                                        :
                       std::make_pair(sell_price.quote.symbol, sell_price.base.symbol);
            }

            asset amount_for_sale() const {
                return asset(for_sale, sell_price.base.symbol);
            }

            asset amount_to_receive() const {
                return amount_for_sale() * sell_price;
            }
        };

/**
 * @class call_order_object
 * @brief tracks debt and call price information
 *
 * There should only be one call_order_object per asset pair per account and
 * they will all have the same call price.
 */
        class call_order_object
                : public object<call_order_object_type, call_order_object> {
        public:
            template<typename Constructor, typename Allocator>
            call_order_object(Constructor &&c, allocator <Allocator> a) {
                c(*this);
            }

            call_order_object() {

            }

            id_type id;

            asset get_collateral() const {
                return asset(collateral, call_price.base.symbol);
            }

            asset get_debt() const {
                return asset(debt, debt_type());
            }

            asset amount_to_receive() const {
                return get_debt();
            }

            asset_symbol_type debt_type() const {
                return call_price.quote.symbol;
            }

            price collateralization() const {
                return get_collateral() / get_debt();
            }

            account_name_type borrower;
            share_type collateral;  ///< call_price.base.asset_id, access via get_collateral
            share_type debt;        ///< call_price.quote.asset_id, access via get_collateral
            price call_price;  ///< Debt / Collateral

            pair <asset_symbol_type, asset_symbol_type> get_market() const {
                auto tmp = std::make_pair(call_price.base.symbol, call_price.quote.symbol);
                if (tmp.first > tmp.second) {
                    std::swap(tmp.first, tmp.second);
                }
                return tmp;
            }
        };

/**
 *  @brief tracks bitassets scheduled for force settlement at some point in the future.
 *
 *  On the @ref settlement_date the @ref balance will be converted to the collateral asset
 *  and paid to @ref owner and then this object will be deleted.
 */
        class force_settlement_object
                : public object<force_settlement_object_type, force_settlement_object> {
        public:
            template<typename Constructor, typename Allocator>
            force_settlement_object(Constructor &&c, allocator <Allocator> a) {
                c(*this);
            }

            force_settlement_object() {

            }

            id_type id;

            account_name_type owner;
            asset balance;
            time_point_sec settlement_date;

            asset_symbol_type settlement_asset_symbol() const {
                return balance.symbol;
            }
        };

        struct by_price;
        struct by_expiration;
        struct by_account;
        typedef multi_index_container <
        limit_order_object,
        indexed_by<
                ordered_unique < tag <
                by_id>, member<limit_order_object, limit_order_object::id_type, &limit_order_object::id>>,
        ordered_non_unique <tag<by_expiration>, member<limit_order_object, time_point_sec, &limit_order_object::expiration>>,
        ordered_unique <tag<by_price>,
        composite_key<limit_order_object,
                member <
                limit_order_object, price, &limit_order_object::sell_price>,
        member<limit_order_object, limit_order_object::id_type, &limit_order_object::id>
        >,
        composite_key_compare <std::greater<price>, std::less<order_id_type>>
        >,
        ordered_unique <tag<by_account>,
        composite_key<limit_order_object,
                member <
                limit_order_object, account_name_type, &limit_order_object::seller>,
        member<limit_order_object, uint32_t, &limit_order_object::orderid>
        >
        >
        >,
        allocator <limit_order_object>
        >
        limit_order_index;

        struct by_collateral;
        struct by_account;
        struct by_price;
        typedef multi_index_container <
        call_order_object,
        indexed_by<
                ordered_unique < tag < by_id>,
        member<call_order_object, call_order_object::id_type, &call_order_object::id>>,
        ordered_unique <tag<by_price>,
        composite_key<call_order_object,
                member <
                call_order_object, price, &call_order_object::call_price>,
        member<call_order_object, call_order_object::id_type, &call_order_object::id>
        >,
        composite_key_compare <std::less<price>, std::less<call_order_object::id_type>>
        >,
        ordered_unique <tag<by_account>,
        composite_key<call_order_object,
                member <
                call_order_object, account_name_type, &call_order_object::borrower>,
        const_mem_fun<call_order_object, asset_symbol_type, &call_order_object::debt_type>
        >
        >,
        ordered_unique <tag<by_collateral>,
        composite_key<call_order_object,
                const_mem_fun <
                call_order_object, price, &call_order_object::collateralization>,
        member<call_order_object, call_order_object::id_type, &call_order_object::id>
        >
        >
        >
        >
        call_order_index;

        struct by_expiration;
        typedef multi_index_container <
        force_settlement_object,
        indexed_by<
                ordered_unique < tag <
                by_id>, member<force_settlement_object, force_settlement_object::id_type, &force_settlement_object::id>>,
        ordered_unique <tag<by_account>,
        composite_key<force_settlement_object,
                member <
                force_settlement_object, account_name_type, &force_settlement_object::owner>,
        member<force_settlement_object, force_settlement_object::id_type, &force_settlement_object::id>
        >
        >,
        ordered_unique <tag<by_expiration>,
        composite_key<force_settlement_object,
                const_mem_fun <
                force_settlement_object, asset_symbol_type, &force_settlement_object::settlement_asset_symbol>,
        member<force_settlement_object, time_point_sec, &force_settlement_object::settlement_date>,
        member<force_settlement_object, force_settlement_object::id_type, &force_settlement_object::id>
        >
        >
        >
        >
        force_settlement_index;
    }
} // steemit::chain

FC_REFLECT(steemit::chain::limit_order_object,
        (id)(created)(expiration)(seller)(orderid)(for_sale)(sell_price))
CHAINBASE_SET_INDEX_TYPE(steemit::chain::limit_order_object, steemit::chain::limit_order_index)

FC_REFLECT(steemit::chain::call_order_object, (borrower)(collateral)(debt)(call_price))
CHAINBASE_SET_INDEX_TYPE(steemit::chain::call_order_object, steemit::chain::call_order_index)

FC_REFLECT(steemit::chain::force_settlement_object, (owner)(balance)(settlement_date))
CHAINBASE_SET_INDEX_TYPE(steemit::chain::force_settlement_object, steemit::chain::force_settlement_index)