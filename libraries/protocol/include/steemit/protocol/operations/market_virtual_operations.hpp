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

            fill_order_operation(const string &c_o, uint32_t c_id, const asset &c_p, const string &o_o, uint32_t o_id, const asset &o_p)
                    : current_owner(c_o), current_order_id(c_id),
                      current_pays(c_p), open_owner(o_o), open_order_id(o_id),
                      open_pays(o_p) {
            }

            account_name_type current_owner;
            integral_id_type current_order_id = 0;
            asset current_pays;
            account_name_type open_owner;
            integral_id_type open_order_id = 0;
            asset open_pays;
        };


        struct fill_asset_order_operation : public virtual_operation {
            fill_asset_order_operation() {

            }

            fill_asset_order_operation(integral_id_type o, account_name_type a, asset p, asset r, asset f)
                    : order_id(o), account_id(a), pays(p), receives(r) {
            }

            integral_id_type order_id;
            account_name_type account_id;
            asset pays;
            asset receives;

            pair<asset_symbol_type, asset_symbol_type> get_market() const {
                return pays.symbol < receives.symbol ?
                       std::make_pair(pays.symbol, receives.symbol) :
                       std::make_pair(receives.symbol, pays.symbol);
            }
        };
    }
}

FC_REFLECT(steemit::protocol::fill_order_operation, (current_owner)(current_order_id)(current_pays)(open_owner)(open_order_id)(open_pays))

FC_REFLECT(steemit::protocol::fill_asset_order_operation, (order_id)(account_id)(pays)(receives))

#endif //GOLOS_MARKET_VIRTUAL_OPERATIONS_HPP