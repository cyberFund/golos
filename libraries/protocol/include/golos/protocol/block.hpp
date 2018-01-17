#pragma once

#include <golos/protocol/block_header.hpp>
#include <golos/protocol/transaction.hpp>

namespace golos {
    namespace protocol {

        /**
         * @brief Tracks signed blocks with transactions
         */
        struct signed_block : public signed_block_header {
            checksum_type calculate_merkle_root() const;

            std::vector<signed_transaction> transactions; ///< Vector of @ref signed_transaction contained in block
        };

    }
} // golos::protocol

FC_REFLECT_DERIVED((golos::protocol::signed_block), ((golos::protocol::signed_block_header)), (transactions))
