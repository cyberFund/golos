#ifndef GOLOS_MARKET_VIRTUAL_OPERATIONS_HPP
#define GOLOS_MARKET_VIRTUAL_OPERATIONS_HPP

#include <steemit/protocol/asset.hpp>
#include <steemit/protocol/base.hpp>
#include <steemit/protocol/block_header.hpp>

namespace steemit {
    namespace protocol {
        /**
         * @ingroup operations
         *
         * @note This is a virtual operation that is created while matching orders and
         * emitted for the purpose of accurately tracking account history, accelerating
         * a reindex.
         */
        struct fill_order_operation : public virtual_operation {
            fill_order_operation() {
            }

            fill_order_operation(const string &c_o, order_id_type c_id, const asset &c_p, const string &o_o, uint32_t o_id, const asset &o_p)
                    : current_owner(c_o), current_order_id(c_id),
                      current_pays(c_p), open_owner(o_o), open_order_id(o_id),
                      open_pays(o_p) {
            }

            account_name_type current_owner;
            order_id_type current_order_id = 0;
            asset current_pays;
            account_name_type open_owner;
            order_id_type open_order_id = 0;
            asset open_pays;
        };

        /**
         * @ingroup operations
         *
         * @note This is a virtual operation that is created while matching orders and
         * emitted for the purpose of accurately tracking account history, accelerating
         * a reindex.
         */
        struct asset_fill_order_operation : public virtual_operation {
            struct fee_parameters_type {

            };

            asset_fill_order_operation() {
            }

            asset_fill_order_operation(order_id_type o, account_name_type a, asset p, asset r, asset f)
                    : order_id(o), account_name(a), pays(p), receives(r),
                      fee(f) {

            }

            order_id_type order_id;
            account_name_type account_name;
            asset pays;
            asset receives;
            asset fee; // paid by receiving account

            pair <asset_symbol_type, asset_symbol_type> get_market() const {
                return pays.symbol < receives.symbol ?
                       std::make_pair(pays.symbol, receives.symbol) :
                       std::make_pair(receives.symbol, pays.symbol);
            }

            account_name_type fee_payer() const {
                return account_name;
            }

            /// This is a virtual operation; there is no fee
            share_type calculate_fee(const fee_parameters_type &k) const {
                return 0;
            }
        };
    }
}

FC_REFLECT(steemit::protocol::fill_order_operation, (current_owner)(current_orderid)(current_pays)(open_owner)(open_orderid)(open_pays))

#endif //GOLOS_MARKET_VIRTUAL_OPERATIONS_HPP