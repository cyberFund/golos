#pragma once

#include <steemit/chain/utilities/asset.hpp>

#include <steemit/protocol/config.hpp>

namespace steemit {
    namespace chain {
        namespace utilities {
            inline bool is_comment_payout_dust(const price &p, uint64_t steem_payout) {
                return to_sbd(p, asset(steem_payout, STEEM_SYMBOL)) <
                       STEEMIT_MIN_PAYOUT_SBD;
            }
        }
    }
}