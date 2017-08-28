#ifndef GOLOS_ASSET_VIRTUAL_OPERATIONS_HPP
#define GOLOS_ASSET_VIRTUAL_OPERATIONS_HPP

#include <steemit/protocol/asset.hpp>
#include <steemit/protocol/base.hpp>

namespace steemit {
    namespace protocol {
        /**
         * Virtual op generated when force settlement is cancelled.
         */

        struct asset_settle_cancel_operation : public virtual_operation {
            integral_id_type settlement;
            /// Account requesting the force settlement. This account pays the fee
            account_name_type account;
            /// Amount of asset to force settle. This must be a market-issued asset
            asset amount;
            extensions_type extensions;

            void validate() const {
                FC_ASSERT(amount.amount > 0, "Must settle at least 1 unit");
            }
        };
    }
}

FC_REFLECT(steemit::protocol::asset_settle_cancel_operation, (settlement)(account)(amount)(extensions))

#endif //GOLOS_ASSET_VIRTUAL_OPERATIONS_HPP
