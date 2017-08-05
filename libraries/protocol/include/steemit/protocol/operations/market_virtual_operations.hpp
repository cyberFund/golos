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

            fill_order_operation(const string &c_o, uint32_t c_id, const asset &c_p, const string &o_o, uint32_t o_id,
                                 const asset &o_p) : current_owner(c_o), current_orderid(c_id), current_pays(c_p),
                                                     open_owner(o_o), open_orderid(o_id), open_pays(o_p) {
            }

            account_name_type current_owner;
            integral_id_type current_orderid = 0;
            asset current_pays;
            account_name_type open_owner;
            integral_id_type open_orderid = 0;
            asset open_pays;
        };

        /**
         * @ingroup operations
         *
         * @note This is a virtual operation that is created while matching call orders and
         * emitted for the purpose of accurately tracking account history, accelerating
         * a reindex.
         */
        struct fill_call_order_operation : public base_operation {
            fill_call_order_operation() {

            }

            fill_call_order_operation(integral_id_type o, const account_name_type &a, const asset &p, const asset &r,
                                      const asset &f) : orderid(o), owner(a), pays(p), receives(r), fee(f) {
            }

            integral_id_type orderid;
            account_name_type owner;
            asset pays;
            asset receives;
            asset fee; // paid by receiving account

            pair <asset_name_type, asset_name_type> get_market() const {
                return pays.symbol < receives.symbol ? std::make_pair(pays.symbol_name(), receives.symbol_name())
                                                     : std::make_pair(receives.symbol_name(), pays.symbol_name());
            }
        };

        /**
         * @ingroup operations
         *
         * @note This is a virtual operation that is created while matching call orders and
         * emitted for the purpose of accurately tracking account history, accelerating
         * a reindex.
         */
        struct fill_settlement_order_operation : public base_operation {
            fill_settlement_order_operation() {

            }

            fill_settlement_order_operation(integral_id_type o, const account_name_type &a, const asset &p,
                                            const asset &r, const asset &f) : orderid(o), owner(a), pays(p),
                                                                              receives(r) {
            }

            integral_id_type orderid;
            account_name_type owner;
            asset pays;
            asset receives;
            asset fee; // paid by receiving account

            pair <asset_name_type, asset_name_type> get_market() const {
                return pays.symbol < receives.symbol ? std::make_pair(pays.symbol_name(), receives.symbol_name())
                                                     : std::make_pair(receives.symbol_name(), pays.symbol_name());
            }
        };

        typedef fc::static_variant<protocol::fill_order_operation, protocol::fill_call_order_operation,
                protocol::fill_settlement_order_operation> market_virtual_operations;
    }
}

FC_REFLECT_TYPENAME(steemit::protocol::market_virtual_operations)

FC_REFLECT(steemit::protocol::fill_order_operation,
           (current_owner)(current_orderid)(current_pays)(open_owner)(open_orderid)(open_pays))
FC_REFLECT(steemit::protocol::fill_call_order_operation, (orderid)(owner)(pays)(receives)(fee))
FC_REFLECT(steemit::protocol::fill_settlement_order_operation, (orderid)(owner)(pays)(receives)(fee))

#endif //GOLOS_MARKET_VIRTUAL_OPERATIONS_HPP