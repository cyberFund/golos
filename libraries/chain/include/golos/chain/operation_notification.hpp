#pragma once

#include <golos/protocol/operations/operations.hpp>

#include <golos/chain/steem_object_types.hpp>

namespace golos {
    namespace chain {

        /**
         * @brief Operation notification context structure
         *
         * @param trx_id Unique transaction identifier
         * @param block block number which contains the transaction with operation context
         * @param trx_in_block amount of transactions in block
         * @param op_in_trx operations amount in transaction
         * @param virtual_op virtual operations amount in transaction
         * @param op @ref operation reference
         *
         * Operation notification context is required to induce @ref boost::signal2 notifications for connected @ref plugin slots. Operation notification context passes the operation coming in database from chain internals to plugin interfaces.
         */
        struct operation_notification {
            operation_notification(const operation &o) : op(o) {
            }

            transaction_id_type trx_id;
            uint32_t block = 0;
            uint32_t trx_in_block = 0;
            uint16_t op_in_trx = 0;
            uint64_t virtual_op = 0;
            const operation &op;
        };

    }
}
