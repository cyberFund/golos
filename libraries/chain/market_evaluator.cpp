#include <steemit/chain/account_object.hpp>
#include <steemit/chain/asset_object.hpp>
#include <steemit/chain/database_exceptions.hpp>
#include <steemit/chain/market_object.hpp>
#include <steemit/chain/market_evaluator.hpp>

#include <steemit/chain/database.hpp>
#include <steemit/chain/hardfork.hpp>

#include <steemit/protocol/exceptions.hpp>
#include <steemit/protocol/operations/market_operations.hpp>

#include <fc/uint128.hpp>
#include <fc/smart_ref_impl.hpp>

namespace steemit {
    namespace chain {
        /// TODO: after the hardfork, we can rename this method validate_permlink because it is strictily less restrictive than before
        ///  Issue #56 contains the justificiation for allowing any UTF-8 string to serve as a permlink, content will be grouped by tags
        ///  going forward.
        inline void validate_permlink(const string &permlink) {
            FC_ASSERT(permlink.size() < STEEMIT_MAX_PERMLINK_LENGTH, "permlink is too long");
            FC_ASSERT(fc::is_utf8(permlink), "permlink not formatted in UTF8");
        }

        inline void validate_account_name(const string &name) {
            FC_ASSERT(is_valid_account_name(name), "Account name ${n} is invalid", ("n", name));
        }

        bool inline is_asset_type(asset asset, asset_symbol_type symbol) {
            return asset.symbol == symbol;
        }

        void convert_evaluator::do_apply(const convert_operation &o) {

            const auto &owner = db.get_account(o.owner);
            FC_ASSERT(db.get_balance(owner, o.amount.symbol_name()) >= o.amount,
                      "Account ${n} does not have sufficient balance for conversion. Balance: ${b}. Required: ${r}",
                      ("n", o.owner)("b", db.get_balance(owner, o.amount.symbol_name()))("r", o.amount));

            db.adjust_balance(owner, -o.amount);

            const auto &fhistory = db.get_feed_history();
            FC_ASSERT(!fhistory.current_median_history.is_null(), "Cannot convert SBD because there is no price feed.");

            auto steem_conversion_delay = STEEMIT_CONVERSION_DELAY_PRE_HF16;
            if (db.has_hardfork(STEEMIT_HARDFORK_0_16__551)) {
                steem_conversion_delay = STEEMIT_CONVERSION_DELAY;
            }

            db.create<convert_request_object>([&](convert_request_object &obj) {
                obj.owner = o.owner;
                obj.requestid = o.request_id;
                obj.amount = o.amount;
                obj.conversion_date = db.head_block_time() + steem_conversion_delay;
            });

        }

        void limit_order_create_evaluator::do_apply(const limit_order_create_operation &op) {
            if (db.has_hardfork(STEEMIT_HARDFORK_0_17__115)) {
                try {
                    const database &d = get_database();

                    FC_ASSERT(op.expiration >= d.head_block_time());

                    seller = d.find_account(op.owner);
                    sell_asset = d.find_asset(op.amount_to_sell.symbol_name());
                    receive_asset = d.find_asset(op.min_to_receive.symbol_name());

                    if (!sell_asset->options.whitelist_markets.empty()) {
                        FC_ASSERT(sell_asset->options.whitelist_markets.find(receive_asset->asset_name) !=
                                  sell_asset->options.whitelist_markets.end());
                    }
                    if (!sell_asset->options.blacklist_markets.empty()) {
                        FC_ASSERT(sell_asset->options.blacklist_markets.find(receive_asset->asset_name) ==
                                  sell_asset->options.blacklist_markets.end());
                    }

                    FC_ASSERT(d.is_authorized_asset(*seller, *sell_asset));
                    FC_ASSERT(d.is_authorized_asset(*seller, *receive_asset));

                    FC_ASSERT(d.get_balance(*seller, *sell_asset) >= op.amount_to_sell, "insufficient balance",
                              ("balance", d.get_balance(*seller, *sell_asset))("amount_to_sell", op.amount_to_sell));

                } FC_CAPTURE_AND_RETHROW((op))

                try {
                    const auto &seller_stats = db.get_account_statistics(seller->name);
                    db.modify(seller_stats, [&](account_statistics_object &bal) {
                        if (op.amount_to_sell.symbol == STEEM_SYMBOL) {
                            bal.total_core_in_orders += op.amount_to_sell.amount;
                        }
                    });

                    db.adjust_balance(db.get_account(op.owner), -op.amount_to_sell);

                    bool filled = db.apply_order(db.create<limit_order_object>([&](limit_order_object &obj) {
                        obj.created = db.head_block_time();
                        obj.order_id = op.order_id;
                        obj.seller = seller->name;
                        obj.for_sale = op.amount_to_sell.amount;
                        obj.sell_price = op.get_price();
                        obj.expiration = op.expiration;
                        obj.deferred_fee = deferred_fee;
                    }));

                    FC_ASSERT(!op.fill_or_kill || filled);
                } FC_CAPTURE_AND_RETHROW((op))
            } else {
                FC_ASSERT((is_asset_type(op.amount_to_sell, STEEM_SYMBOL) &&
                           is_asset_type(op.min_to_receive, SBD_SYMBOL)) ||
                          (is_asset_type(op.amount_to_sell, SBD_SYMBOL) &&
                           is_asset_type(op.min_to_receive, STEEM_SYMBOL)),
                          "Limit order must be for the STEEM:SBD market");

                FC_ASSERT(op.expiration > db.head_block_time(),
                          "Limit order has to expire after head block time.");

                const auto &owner = db.get_account(op.owner);

                FC_ASSERT(db.get_balance(owner, op.amount_to_sell.symbol_name()) >= op.amount_to_sell,
                          "Account does not have sufficient funds for limit order.");

                db.adjust_balance(owner, -op.amount_to_sell);

                const auto &order = db.create<limit_order_object>([&](limit_order_object &obj) {
                    obj.created = db.head_block_time();
                    obj.seller = op.owner;
                    obj.order_id = op.order_id;
                    obj.for_sale = op.amount_to_sell.amount;
                    obj.sell_price = op.get_price();
                    obj.expiration = op.expiration;
                });

                bool filled = db.apply_order(order);

                if (op.fill_or_kill) {
                    FC_ASSERT(filled, "Cancelling order because it was not filled.");
                }
            }
        }

        void limit_order_create2_evaluator::do_apply(const limit_order_create2_operation &op) {
            if (db.has_hardfork(STEEMIT_HARDFORK_0_17__115)) {
                try {
                    const database &d = get_database();

                    FC_ASSERT(op.expiration >= d.head_block_time());

                    seller = d.find_account(op.owner);
                    sell_asset = d.find_asset(op.amount_to_sell.symbol_name());
                    receive_asset = d.find_asset(op.exchange_rate.quote.symbol_name());

                    if (!sell_asset->options.whitelist_markets.empty()) {
                        FC_ASSERT(sell_asset->options.whitelist_markets.find(receive_asset->asset_name) !=
                                  sell_asset->options.whitelist_markets.end());
                    }
                    if (!sell_asset->options.blacklist_markets.empty()) {
                        FC_ASSERT(sell_asset->options.blacklist_markets.find(receive_asset->asset_name) ==
                                  sell_asset->options.blacklist_markets.end());
                    }

                    FC_ASSERT(d.is_authorized_asset(*seller, *sell_asset));
                    FC_ASSERT(d.is_authorized_asset(*seller, *receive_asset));

                    FC_ASSERT(d.get_balance(*seller, *sell_asset) >= op.amount_to_sell, "insufficient balance",
                              ("balance", d.get_balance(*seller, *sell_asset))("amount_to_sell", op.amount_to_sell));

                } FC_CAPTURE_AND_RETHROW((op))

                try {
                    const auto &seller_stats = db.get_account_statistics(seller->name);
                    get_database().modify(seller_stats, [&](account_statistics_object &bal) {
                        if (op.amount_to_sell.symbol == STEEM_SYMBOL) {
                            bal.total_core_in_orders += op.amount_to_sell.amount;
                        }
                    });

                    get_database().adjust_balance(get_database().get_account(op.owner), -op.amount_to_sell);

                    bool filled = db.apply_order(db.create<limit_order_object>([&](limit_order_object &obj) {
                        obj.created = db.head_block_time();
                        obj.order_id = op.order_id;
                        obj.seller = seller->name;
                        obj.for_sale = op.amount_to_sell.amount;
                        obj.sell_price = op.get_price();
                        obj.expiration = op.expiration;
                        obj.deferred_fee = deferred_fee;
                    }));

                    FC_ASSERT(!op.fill_or_kill || filled);
                } FC_CAPTURE_AND_RETHROW((op))
            } else {
                FC_ASSERT((is_asset_type(op.amount_to_sell, STEEM_SYMBOL) &&
                           is_asset_type(op.exchange_rate.quote, SBD_SYMBOL)) ||
                          (is_asset_type(op.amount_to_sell, SBD_SYMBOL) &&
                           is_asset_type(op.exchange_rate.quote, STEEM_SYMBOL)),
                          "Limit order must be for the STEEM:SBD market");

                FC_ASSERT(op.expiration > db.head_block_time(),
                          "Limit order has to expire after head block time.");

                const auto &owner = db.get_account(op.owner);

                FC_ASSERT(db.get_balance(owner, op.amount_to_sell.symbol_name()) >= op.amount_to_sell,
                          "Account does not have sufficient funds for limit order.");

                db.adjust_balance(owner, -op.amount_to_sell);

                const auto &order = db.create<limit_order_object>([&](limit_order_object &obj) {
                    obj.created = db.head_block_time();
                    obj.seller = op.owner;
                    obj.order_id = op.order_id;
                    obj.for_sale = op.amount_to_sell.amount;
                    obj.sell_price = op.exchange_rate;
                    obj.expiration = op.expiration;
                });

                bool filled = db.apply_order(order);

                if (op.fill_or_kill) {
                    FC_ASSERT(filled, "Cancelling order because it was not filled.");
                }
            }
        }

        void limit_order_cancel_evaluator::do_apply(const limit_order_cancel_operation &op) {
            if (db.has_hardfork(STEEMIT_HARDFORK_0_17__115)) {
                try {
                    database &d = get_database();

                    _order = d.find_limit_order(op.owner, op.order_id);
                    FC_ASSERT(_order->seller == op.owner);
                } FC_CAPTURE_AND_RETHROW((op))

                try {
                    database &d = get_database();

                    auto base_asset = _order->sell_price.base.symbol_name();
                    auto quote_asset = _order->sell_price.quote.symbol_name();

                    d.cancel_order(*_order, false /* don't create a virtual op*/);

                    // Possible optimization: order can be called by canceling a limit order iff the canceled order was at the top of the book.
                    // Do I need to check calls in both assets?
                    d.check_call_orders(d.get_asset(base_asset));
                    d.check_call_orders(d.get_asset(quote_asset));
                } FC_CAPTURE_AND_RETHROW((op))
            } else {
                db.cancel_order(db.get_limit_order(op.owner, op.order_id), false);
            }
        }

        void call_order_update_evaluator::do_apply(const call_order_update_operation &op) {
            try {
                _paying_account = db.find_account(op.funding_account);
                _debt_asset = db.find_asset(op.delta_debt.symbol_name());

                FC_ASSERT(_debt_asset->is_market_issued(),
                          "Unable to cover ${sym} as it is not a collateralized asset.",
                          ("sym", _debt_asset->asset_name));

                _bitasset_data = db.find_asset_bitasset_data(_debt_asset->asset_name);

                /// if there is a settlement for this asset, then no further margin positions may be taken and
                /// all existing margin positions should have been closed va database::globally_settle_asset
                FC_ASSERT(!_bitasset_data->has_settlement());

                FC_ASSERT(op.delta_collateral.symbol_name() == _bitasset_data->options.short_backing_asset);

                if (_bitasset_data->is_prediction_market) {
                    FC_ASSERT(op.delta_collateral.amount == op.delta_debt.amount);
                } else if (_bitasset_data->current_feed.settlement_price.is_null()) {
                    FC_THROW_EXCEPTION(insufficient_feeds, "Cannot borrow asset with no price feed.");
                }

                if (op.delta_debt.amount < 0) {
                    FC_ASSERT(db.get_balance(*_paying_account, *_debt_asset) >= op.delta_debt,
                              "Cannot cover by ${c} when payer only has ${b}",
                              ("c", op.delta_debt.amount)("b", db.get_balance(*_paying_account, *_debt_asset).amount));
                }

                if (op.delta_collateral.amount > 0) {
                    FC_ASSERT(
                            db.get_balance(*_paying_account, db.get_asset(_bitasset_data->options.short_backing_asset)) >=
                            op.delta_collateral, "Cannot increase collateral by ${c} when payer only has ${b}",
                            ("c", op.delta_collateral.amount)("b", db.get_balance(*_paying_account, db.get_asset(
                                    op.delta_collateral.symbol_name())).amount));
                }
            } FC_CAPTURE_AND_RETHROW((op))

            try {
                database &d = get_database();

                if (op.delta_debt.amount != 0) {
                    d.adjust_balance(d.get_account(op.funding_account), op.delta_debt);

                    // Deduct the debt paid from the total supply of the debt asset.
                    d.modify(d.get_asset_dynamic_data(_debt_asset->asset_name),
                             [&](asset_dynamic_data_object &dynamic_asset) {
                                 dynamic_asset.current_supply += op.delta_debt.amount;
                                 assert(dynamic_asset.current_supply >= 0);
                             });
                }

                if (op.delta_collateral.amount != 0) {
                    d.adjust_balance(d.get_account(op.funding_account), -op.delta_collateral);

                    // Adjust the total core in orders accodingly
                    if (op.delta_collateral.symbol == STEEM_SYMBOL) {
                        d.modify(d.get_account_statistics(_paying_account->name),
                                 [&](account_statistics_object &stats) {
                                     stats.total_core_in_orders += op.delta_collateral.amount;
                                 });
                    }
                }


                auto &call_idx = d.get_index<call_order_index>().indices().get<by_account>();
                auto itr = call_idx.find(boost::make_tuple(op.funding_account, op.delta_debt.symbol_name()));
                const call_order_object *call_obj = nullptr;

                if (itr == call_idx.end()) {
                    FC_ASSERT(op.delta_collateral.amount > 0);
                    FC_ASSERT(op.delta_debt.amount > 0);

                    call_obj = &d.create<call_order_object>([&](call_order_object &call) {
                        call.order_id = op.order_id;
                        call.borrower = op.funding_account;
                        call.collateral = op.delta_collateral.amount;
                        call.debt = op.delta_debt.amount;
                        call.call_price = price::call_price(op.delta_debt, op.delta_collateral,
                                                            _bitasset_data->current_feed.maintenance_collateral_ratio);

                    });
                } else {
                    call_obj = &*itr;

                    d.modify(*call_obj, [&](call_order_object &call) {
                        call.collateral += op.delta_collateral.amount;
                        call.debt += op.delta_debt.amount;
                        if (call.debt > 0) {
                            call.call_price = price::call_price(call.get_debt(), call.get_collateral(),
                                                                _bitasset_data->current_feed.maintenance_collateral_ratio);
                        }
                    });
                }

                auto debt = call_obj->get_debt();
                if (debt.amount == 0) {
                    FC_ASSERT(call_obj->collateral == 0);
                    d.remove(*call_obj);
                    return void();
                }

                FC_ASSERT(call_obj->collateral > 0 && call_obj->debt > 0);

                // then we must check for margin calls and other issues
                if (!_bitasset_data->is_prediction_market) {
                    call_order_object::id_type call_order_id = call_obj->id;

                    // check to see if the order needs to be margin called now, but don't allow black swans and require there to be
                    // limit orders available that could be used to fill the order.
                    if (d.check_call_orders(*_debt_asset, false)) {
                        const auto order_obj = d.find<call_order_object, by_id>(call_order_id);
                        // if we filled at least one call order, we are OK if we totally filled.
                        STEEMIT_ASSERT(!order_obj, call_order_update_unfilled_margin_call,
                                       "Updating call order would trigger a margin call that cannot be fully filled",
                                       ("a", ~order_obj->call_price)("b",
                                                                     _bitasset_data->current_feed.settlement_price));
                    } else {
                        const auto order_obj = d.find<call_order_object, by_id>(call_order_id);
                        FC_ASSERT(order_obj, "no margin call was executed and yet the call object was deleted");
                        //edump( (~order_obj->call_price) ("<")( _bitasset_data->current_feed.settlement_price) );
                        // We didn't fill any call orders.  This may be because we
                        // aren't in margin call territory, or it may be because there
                        // were no matching orders.  In the latter case, we throw.
                        STEEMIT_ASSERT(~order_obj->call_price < _bitasset_data->current_feed.settlement_price,
                                       call_order_update_unfilled_margin_call,
                                       "Updating call order would trigger a margin call that cannot be fully filled",
                                       ("a", ~order_obj->call_price)("b",
                                                                     _bitasset_data->current_feed.settlement_price));
                    }
                }
            } FC_CAPTURE_AND_RETHROW((op))
        }

        void bid_collateral_evaluator::do_apply(const bid_collateral_operation &o) {
            const account_object &paying_account = db.get_account(o.bidder);

            try {
                const asset_object &debt_asset = db.get_asset(o.debt_covered.symbol_name());
                FC_ASSERT(debt_asset.is_market_issued(),
                          "Unable to cover ${sym} as it is not a collateralized asset.",
                          ("sym", debt_asset.asset_name));

                const asset_bitasset_data_object &bitasset_data = db.get_asset_bitasset_data(debt_asset.asset_name);

                FC_ASSERT(bitasset_data.has_settlement());

                FC_ASSERT(o.additional_collateral.symbol_name() == bitasset_data.options.short_backing_asset);

                FC_ASSERT(!bitasset_data.is_prediction_market, "Cannot bid on a prediction market!");

                if (o.additional_collateral.amount > 0) {
                    FC_ASSERT(db.get_balance(paying_account, db.get_asset_bitasset_data(
                            bitasset_data.options.short_backing_asset).asset_name) >= o.additional_collateral,
                              "Cannot bid ${c} collateral when payer only has ${b}",
                              ("c", o.additional_collateral.amount)("b", db.get_balance(paying_account, db.get_asset(
                                      o.additional_collateral.symbol_name())).amount));
                }

                const auto &index = db.get_index<collateral_bid_index>().indices().get<by_account>();
                const auto &bid = index.find(boost::make_tuple(o.debt_covered.symbol_name(), o.bidder));
                if (bid != index.end()) {
                    _bid = &(*bid);
                } else
                    FC_ASSERT(o.debt_covered.amount > 0, "Can't find bid to cancel?!");
            } FC_CAPTURE_AND_RETHROW((o))


            try {
                if (_bid != nullptr) {
                    db.cancel_bid(*_bid, false);
                }

                if (o.debt_covered.amount == 0) {
                    return;
                }

                db.adjust_balance(paying_account, -o.additional_collateral);
                _bid = &db.create<collateral_bid_object>([&](collateral_bid_object &bid) {
                    bid.bidder = o.bidder;
                    bid.inv_swan_price = o.additional_collateral / o.debt_covered;
                });
            } FC_CAPTURE_AND_RETHROW((o))
        }
    }
}