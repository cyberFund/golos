#include <steemit/market_history/key_interface.hpp>

namespace steemit {
    namespace market_history {
        key_interface::key_interface() {

        }

        key_interface::key_interface(protocol::asset_name_type base, protocol::asset_name_type quote)
                : base(base), quote(quote) {
        }
    }
}