#pragma once

#include <steemit/protocol/asset.hpp>

namespace steemit {
namespace market_history {
class key_interface {
        public:
            key_interface();

            key_interface(protocol::asset_symbol_type base, protocol::asset_symbol_type quote);

            protocol::asset_symbol_type base;
            protocol::asset_symbol_type quote;
        };
}
}

FC_REFLECT(steemit::market_history::key_interface, (base)(quote));