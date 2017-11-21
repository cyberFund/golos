#pragma once

#include <golos/protocol/types.hpp>

#include <fc/uint128_t.hpp>

namespace golos {
    namespace chain {
        namespace utilities {
            inline boost::multiprecision::uint256_t to256(const fc::uint128_t &t) {
                boost::multiprecision::uint256_t v(t.hi);
                v <<= 64;
                v += t.lo;
                return v;
            }
        }
    }
}
