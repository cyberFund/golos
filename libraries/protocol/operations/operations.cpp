#include <steemit/protocol/operations/operations.hpp>

#include <steemit/protocol/operations/operation_utilities_impl.hpp>

namespace steemit {
    namespace protocol {

        struct is_market_op_visitor {
            typedef bool result_type;

            template<typename T>
            bool operator()(T &&v) const {
                return false;
            }

            template<uint8_t Major, uint8_t Hardfork, uint16_t Release>
            bool operator()(const limit_order_create_operation<Major, Hardfork, Release> &) const {
                return true;
            }

            template<uint8_t Major, uint8_t Hardfork, uint16_t Release>
            bool operator()(const limit_order_create2_operation<Major, Hardfork, Release> &) const {
                return true;
            }

            template<uint8_t Major, uint8_t Hardfork, uint16_t Release>
            bool operator()(const limit_order_cancel_operation<Major, Hardfork, Release> &) const {
                return true;
            }

            template<uint8_t Major, uint8_t Hardfork, uint16_t Release>
            bool operator()(const call_order_update_operation<Major, Hardfork, Release> &) const {
                return true;
            }

            template<uint8_t Major, uint8_t Hardfork, uint16_t Release>
            bool operator()(const bid_collateral_operation<Major, Hardfork, Release> &) const {
                return true;
            }

            template<uint8_t Major, uint8_t Hardfork, uint16_t Release>
            bool operator()(const transfer_operation<Major, Hardfork, Release> &) const {
                return true;
            }

            template<uint8_t Major, uint8_t Hardfork, uint16_t Release>
            bool operator()(const transfer_to_vesting_operation<Major, Hardfork, Release> &) const {
                return true;
            }
        };

        bool is_market_operation(const operation &op) {
            return op.visit(is_market_op_visitor());
        }

        struct is_vop_visitor {
            typedef bool result_type;

            template<typename T>
            bool operator()(const T &v) const {
                return v.is_virtual();
            }
        };

        bool is_virtual_operation(const operation &op) {
            return op.visit(is_vop_visitor());
        }

    }
} // steemit::protocol

STEEMIT_DEFINE_OPERATION_TYPE(steemit::protocol::operation)
