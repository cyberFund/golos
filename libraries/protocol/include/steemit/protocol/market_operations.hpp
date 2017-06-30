#pragma once

#include <steemit/protocol/base.hpp>
#include <steemit/protocol/market_virtual_operations.hpp>

namespace steemit {
    namespace protocol {

        /**
         *  This operation instructs the blockchain to start a conversion between STEEM and SBD,
         *  The funds are deposited after STEEMIT_CONVERSION_DELAY
         */
        struct convert_operation : public base_operation {
            account_name_type owner;
            order_id_type request_id = 0;
            asset amount;

            void validate() const;

            void get_required_active_authorities(flat_set<account_name_type> &a) const {
                a.insert(owner);
            }
        };

        /**
         *  @class limit_order_create_operation
         *  @brief instructs the blockchain to attempt to sell one asset for another
         *  @ingroup operations
         *
         *  The blockchain will attempt to sell amount_to_sell.symbol for as
         *  much min_to_receive.asset_id as possible.  The fee will be paid by
         *  the seller's account.  Market fees will apply as specified by the
         *  issuer of both the selling asset and the receiving asset as
         *  a percentage of the amount exchanged.
         *
         *  If either the selling asset or the receiving asset is white list
         *  restricted, the order will only be created if the seller is on
         *  the white list of the restricted asset type.
         *
         *  Market orders are matched in the order they are included
         *  in the block chain.
         */
        /**
         * This operation creates a limit order and matches it against existing open orders.
         */
        struct limit_order_create_operation : public base_operation {
            account_name_type owner;
            order_id_type order_id = 0; /// an ID assigned by owner, must be unique
            asset amount_to_sell;
            asset min_to_receive;
            bool fill_or_kill = false;
            time_point_sec expiration = time_point_sec::maximum();

            void validate() const;

            void get_required_active_authorities(flat_set<account_name_type> &a) const {
                a.insert(owner);
            }

            price get_price() const {
                return amount_to_sell / min_to_receive;
            }

            pair<asset_symbol_type, asset_symbol_type> get_market() const {
                return amount_to_sell.symbol < min_to_receive.symbol ?
                       std::make_pair(amount_to_sell.symbol, min_to_receive.symbol)
                                                                     :
                       std::make_pair(min_to_receive.symbol, amount_to_sell.symbol);
            }
        };


        /**
         *  This operation is identical to limit_order_create except it serializes the price rather
         *  than calculating it from other fields.
         */
        struct limit_order_create2_operation : public base_operation {
            account_name_type owner;
            order_id_type order_id = 0; /// an ID assigned by owner, must be unique
            asset amount_to_sell;
            bool fill_or_kill = false;
            price exchange_rate;
            time_point_sec expiration = time_point_sec::maximum();

            void validate() const;

            void get_required_active_authorities(flat_set<account_name_type> &a) const {
                a.insert(owner);
            }

            price get_price() const {
                return exchange_rate;
            }

            pair<asset_symbol_type, asset_symbol_type> get_market() const {
                return exchange_rate.base.symbol < exchange_rate.quote.symbol ?
                       std::make_pair(exchange_rate.base.symbol, exchange_rate.quote.symbol)
                                                                              :
                       std::make_pair(exchange_rate.quote.symbol, exchange_rate.base.symbol);
            }
        };


        /**
         *  @ingroup operations
         *  Used to cancel an existing limit order. Both fee_pay_account and the
         *  account to receive the proceeds must be the same as order->seller.
         *
         *  @return the amount actually refunded
         */
        struct limit_order_cancel_operation : public base_operation {
            account_name_type owner;
            order_id_type order_id = 0;

            void validate() const;

            void get_required_active_authorities(flat_set<account_name_type> &a) const {
                a.insert(owner);
            }
        };


        /**
         *  @ingroup operations
         *
         *  This operation can be used to add collateral, cover, and adjust the margin call price for a particular user.
         *
         *  For prediction markets the collateral and debt must always be equal.
         *
         *  This operation will fail if it would trigger a margin call that couldn't be filled.  If the margin call hits
         *  the call price limit then it will fail if the call price is above the settlement price.
         *
         *  @note this operation can be used to force a market order using the collateral without requiring outside funds.
         */
        struct call_order_update_operation : public base_operation {
            /** this is slightly more expensive than limit orders, this pricing impacts prediction markets */
            struct fee_parameters_type {
                uint64_t fee = 20;
            };

            asset fee;
            account_name_type funding_account; ///< pays fee, collateral, and cover
            asset delta_collateral; ///< the amount of collateral to add to the margin position
            asset delta_debt; ///< the amount of the debt to be paid off, may be negative to issue new debt
            extensions_type extensions;

            account_name_type fee_payer() const {
                return funding_account;
            }

            void validate() const;
        };
    }
} // steemit::protocol

FC_REFLECT(steemit::protocol::limit_order_create_operation::fee_parameters_type, (fee))
FC_REFLECT(steemit::protocol::limit_order_cancel_operation::fee_parameters_type, (fee))
FC_REFLECT(steemit::protocol::call_order_update_operation::fee_parameters_type, (fee))
/// THIS IS THE ONLY VIRTUAL OPERATION THUS FAR...
FC_REFLECT(steemit::protocol::fill_order_operation::fee_parameters_type,)

FC_REFLECT(steemit::protocol::convert_operation, (owner)(requestid)(amount))

FC_REFLECT(steemit::protocol::limit_order_create_operation, (owner)(orderid)(amount_to_sell)(min_to_receive)(fill_or_kill)(expiration))
FC_REFLECT(steemit::protocol::limit_order_create2_operation, (owner)(orderid)(amount_to_sell)(exchange_rate)(fill_or_kill)(expiration))
FC_REFLECT(steemit::protocol::limit_order_cancel_operation, (owner)(orderid))
FC_REFLECT(steemit::protocol::fill_order_operation, (fee)(order_id)(account_id)(pays)(receives))