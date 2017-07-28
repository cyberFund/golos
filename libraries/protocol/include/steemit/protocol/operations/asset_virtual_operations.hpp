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
            struct fee_parameters_type {

            };

            asset fee;
            integral_id_type settlement;
            /// Account requesting the force settlement. This account pays the fee
            account_name_type account;
            /// Amount of asset to force settle. This must be a market-issued asset
            asset amount;
            extensions_type extensions;

            account_name_type fee_payer() const {
                return account;
            }

            void validate() const {
                FC_ASSERT(amount.amount > 0, "Must settle at least 1 unit");
            }

            share_type calculate_fee(const fee_parameters_type &params) const {
                return 0;
            }
        };
    }
}

FC_REFLECT(steemit::protocol::asset_settle_cancel_operation::fee_parameters_type,)
FC_REFLECT(steemit::protocol::asset_settle_cancel_operation, (fee)(settlement)(account)(amount)(extensions))

#endif //GOLOS_ASSET_VIRTUAL_OPERATIONS_HPP
