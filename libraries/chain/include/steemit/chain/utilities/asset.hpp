#pragma once

#include <steemit/protocol/asset.hpp>

namespace steemit {
    namespace chain {
        namespace utilities {

            using steemit::protocol::asset;
            using steemit::protocol::price;

            inline asset to_sbd(const price &p, const asset &steem) {
                FC_ASSERT(steem.symbol == STEEM_SYMBOL_NAME);
                if (p.is_null()) {
                    return asset(0, SBD_SYMBOL_NAME);
                }
                return steem * p;
            }

            inline asset to_steem(const price &p, const asset &sbd) {
                FC_ASSERT(sbd.symbol == SBD_SYMBOL_NAME);
                if (p.is_null()) {
                    return asset(0, STEEM_SYMBOL_NAME);
                }
                return sbd * p;
            }
        }
    }
}