#ifndef GOLOS_MARKET_VIRTUAL_OPERATIONS_HPP
#define GOLOS_MARKET_VIRTUAL_OPERATIONS_HPP

#include <golos/protocol/asset.hpp>
#include <golos/protocol/base.hpp>
#include <golos/protocol/block_header.hpp>

namespace golos {
    namespace protocol {

        /**
         * @ingroup operations
         *
         * @note This is a virtual operation that is created while matching orders and
         * emitted for the purpose of accurately tracking account history, accelerating
         * a reindex.
         */
        template<uint8_t Major, uint8_t Hardfork, uint16_t Release>
        struct fill_order_operation : public virtual_operation<Major, Hardfork, Release> {
            fill_order_operation() {

            }

            fill_order_operation(const std::string &c_o, uint32_t c_id, const asset <Major, Hardfork, Release> &c_p,
                                 const asset <Major, Hardfork, Release> &o_p, const asset <Major, Hardfork, Release> &f)
                    : owner(c_o), order_id(c_id), pays(c_p), receives(o_p), fee(f) {
            }

            account_name_type owner;
            integral_id_type order_id = 0;
            asset <Major, Hardfork, Release> pays;
            asset <Major, Hardfork, Release> receives;
            asset <Major, Hardfork, Release> fee; // paid by receiving account

            std::pair<typename asset<Major, Hardfork, Release>::asset_container_type,
                    typename asset<Major, Hardfork, Release>::asset_container_type> get_market() const {
                return pays.symbol < receives.symbol ? std::make_pair(pays.symbol, receives.symbol) : std::make_pair(
                        receives.symbol, pays.symbol);
            }
        };

        /**
         * @ingroup operations
         *
         * @note This is a virtual operation that is created while matching call orders and
         * emitted for the purpose of accurately tracking account history, accelerating
         * a reindex.
         */
        template<uint8_t Major, uint8_t Hardfork, uint16_t Release>
        struct fill_call_order_operation : public virtual_operation<Major, Hardfork, Release> {
            fill_call_order_operation() {

            }

            fill_call_order_operation(integral_id_type o, const account_name_type &a,
                                      const asset <Major, Hardfork, Release> &p,
                                      const asset <Major, Hardfork, Release> &r,
                                      const asset <Major, Hardfork, Release> &f) : order_id(o), owner(a), pays(p),
                    receives(r), fee(f) {
            }

            integral_id_type order_id;
            account_name_type owner;
            asset <Major, Hardfork, Release> pays;
            asset <Major, Hardfork, Release> receives;
            asset <Major, Hardfork, Release> fee; // paid by receiving account

            std::pair<typename asset<Major, Hardfork, Release>::asset_container_type,
                    typename asset<Major, Hardfork, Release>::asset_container_type> get_market() const {
                return pays.symbol < receives.symbol ? std::make_pair(pays.symbol, receives.symbol) : std::make_pair(
                        receives.symbol, pays.symbol);
            }
        };

        /**
         * @ingroup operations
         *
         * @note This is a virtual operation that is created while matching call orders and
         * emitted for the purpose of accurately tracking account history, accelerating
         * a reindex.
         */
        template<uint8_t Major, uint8_t Hardfork, uint16_t Release>
        struct fill_settlement_order_operation : public virtual_operation<Major, Hardfork, Release> {
            fill_settlement_order_operation() {

            }

            fill_settlement_order_operation(integral_id_type o, const account_name_type &a,
                                            const asset <Major, Hardfork, Release> &p,
                                            const asset <Major, Hardfork, Release> &r,
                                            const asset <Major, Hardfork, Release> &f) : order_id(o), owner(a), pays(p),
                    receives(r) {
            }

            integral_id_type order_id;
            account_name_type owner;
            asset <Major, Hardfork, Release> pays;
            asset <Major, Hardfork, Release> receives;
            asset <Major, Hardfork, Release> fee; // paid by receiving account

            std::pair<typename asset<Major, Hardfork, Release>::asset_container_type,
                    typename asset<Major, Hardfork, Release>::asset_container_type> get_market() const {
                return pays.symbol < receives.symbol ? std::make_pair(pays.symbol, receives.symbol) : std::make_pair(
                        receives.symbol, pays.symbol);
            }
        };

        /**
         * @ingroup operations
         *
         * @note This is a virtual operation that is created while reviving a
         * bitasset from collateral bids.
         */
        template<uint8_t Major, uint8_t Hardfork, uint16_t Release>
        struct execute_bid_operation : public virtual_operation<Major, Hardfork, Release> {
            execute_bid_operation() {
            }

            execute_bid_operation(account_name_type a, const asset <Major, Hardfork, Release> &d,
                                  const asset <Major, Hardfork, Release> &c) : bidder(a), debt(d), collateral(c) {
            }

            account_name_type bidder;
            asset <Major, Hardfork, Release> debt;
            asset <Major, Hardfork, Release> collateral;
        };

        typedef fc::static_variant<protocol::fill_order_operation<0, 16, 0>, protocol::fill_order_operation<0, 17, 0>,
                protocol::fill_call_order_operation<0, 17, 0>, protocol::fill_settlement_order_operation<0, 17, 0>,
                protocol::execute_bid_operation<0, 17, 0>> market_virtual_operations;
    }
}

namespace fc {

    void to_variant(const golos::protocol::market_virtual_operations &, fc::variant &);

    void from_variant(const fc::variant &, golos::protocol::market_virtual_operations &);

} /* fc */

FC_REFLECT_TYPENAME((golos::protocol::market_virtual_operations))

FC_REFLECT((golos::protocol::fill_order_operation<0, 16, 0>), (owner)(order_id)(pays)(receives)(fee))
FC_REFLECT((golos::protocol::fill_order_operation<0, 17, 0>), (owner)(order_id)(pays)(receives)(fee))

FC_REFLECT((golos::protocol::fill_call_order_operation<0, 17, 0>), (order_id)(owner)(pays)(receives)(fee))

FC_REFLECT((golos::protocol::fill_settlement_order_operation<0, 17, 0>), (order_id)(owner)(pays)(receives)(fee))

FC_REFLECT((golos::protocol::execute_bid_operation<0, 17, 0>), (bidder)(debt)(collateral))

#endif //GOLOS_MARKET_VIRTUAL_OPERATIONS_HPP