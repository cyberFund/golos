#pragma once

#include <steemit/protocol/base.hpp>
#include <steemit/protocol/types.hpp>
#include <steemit/protocol/asset.hpp>
#include <boost/preprocessor/seq/seq.hpp>


#include <fc/reflect/reflect.hpp>
#include <fc/crypto/ripemd160.hpp>
#include <fc/time.hpp>

#include <cstdint>
#include <vector>
#include <utility>

#include <steemit/encrypt/crypto_types.hpp>

namespace steemit {
    namespace protocol {

        /**
         * @ingroup transactions
         * @brief This is a virtual operation emitted for the purpose of returning escrow to author
         */
        struct return_escrow_submission_operation : public base_operation {
            account_name_type author;
            asset escrow;
            content_object::id_type content;

            account_name_type fee_payer() const {
                return author;
            }

            void validate() const {
                FC_ASSERT(!"virtual operation");
            }
        };

        /**
         * @ingroup transactions
         * @brief This is a virtual operation emitted for the purpose of returning escrow to consumer
         */
        struct return_escrow_buying_operation : public base_operation {
            account_name_type consumer;
            asset escrow;
            buying_object::id_type buying;

            account_name_type fee_payer() const {
                return consumer;
            }

            void validate() const {
                FC_ASSERT(!"virtual operation");
            }
        };

        /**
         * @ingroup transactions
         * @brief This operation is used to report stats. These stats are later used to rate seeders.
         */
        struct report_stats_operation : public base_operation {
            /// Map of seeders to amount they uploaded
            map<account_name_type, uint64_t> stats;
            account_name_type consumer;

            account_name_type fee_payer() const {
                return consumer;
            }

            void validate() const;
        };

        /**
         * @ingroup transactions
         * @brief
         */
        struct pay_seeder_operation : public base_operation {
            asset payout;
            account_name_type author;
            account_name_type seeder;

            account_name_type fee_payer() const {
                return author;
            }

            void validate() const {
                FC_ASSERT(!"virtual operation");
            }
        };

        /**
         * @ingroup transactions
         * @brief
         */
        struct finish_buying_operation : public base_operation {
            asset payout;
            // do we need here region_code_from?
            account_name_type author;
            map<account_name_type, uint32_t> co_authors;
            account_name_type consumer;
            buying_object::id_type buying;

            account_name_type fee_payer() const {
                return author;
            }

            void validate() const {
                FC_ASSERT(!"virtual operation");
            }
        };
    }
} // steemit::chain

FC_REFLECT(steemit::chain::return_escrow_submission_operation,
           (author)(escrow)(content))
FC_REFLECT(steemit::chain::return_escrow_buying_operation,
           (consumer)(escrow)(buying))
FC_REFLECT(steemit::chain::report_stats_operation, (consumer)(stats))
FC_REFLECT(steemit::chain::pay_seeder_operation, (payout)(author)(seeder));
FC_REFLECT(steemit::chain::finish_buying_operation,
           (payout)(author)(co_authors)(buying)(consumer));