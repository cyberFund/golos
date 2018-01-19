#pragma once

#include <fc/reflect/reflect.hpp>

#include <cstdint>

#include <golos/protocol/config.hpp>

namespace golos {
    namespace chain {

        /**
         * @brief This represents non-mutable compile-time chain parameters
         */
        struct immutable_chain_parameters {
            uint16_t min_committee_member_count = STEEMIT_DEFAULT_MIN_COMMITTEE_MEMBER_COUNT; ///< Default minimum available commitee member count
            uint16_t min_witness_count = STEEMIT_DEFAULT_MIN_WITNESS_COUNT; ///< Default minimum witness amount
            uint32_t num_special_accounts = 0; ///< Indicates special (genesis-created, hardcoded) accounts
            uint32_t num_special_assets = 0; ///< Indicates special (genesis-created, hardcoded) assets
        };

    }
} // golos::chain

FC_REFLECT(golos::chain::immutable_chain_parameters,
        (min_committee_member_count)
                (min_witness_count)
                (num_special_accounts)
                (num_special_assets)
)
