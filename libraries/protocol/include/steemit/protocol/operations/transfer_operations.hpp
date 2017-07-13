#ifndef GOLOS_TRANSFER_OPERATIONS_HPP
#define GOLOS_TRANSFER_OPERATIONS_HPP

#include <steemit/protocol/asset.hpp>
#include <steemit/protocol/base.hpp>
#include <steemit/protocol/block_header.hpp>

namespace steemit {
    namespace protocol {
        /**
         * @ingroup operations
         *
         * @brief Transfers an amount of one asset from one account to another
         *
         *  Fees are paid by the "from" account
         *
         *  @pre amount.amount > 0
         *  @pre fee.amount >= 0
         *  @pre from != to
         *  @post from account's balance will be reduced by fee and amount
         *  @post to account's balance will be increased by amount
         *  @return n/a
         */
        struct transfer_operation : public base_operation {
            account_name_type from;
            /// Account to transfer asset to
            account_name_type to;
            /// The amount of asset to transfer from @ref from to @ref to
            asset amount;

            /// The memo is plain-text, any encryption on the memo is up to
            /// a higher level protocol.
            string memo;

            void validate() const;

            void get_required_active_authorities(flat_set<account_name_type> &a) const {
                if (amount.symbol != VESTS_SYMBOL) {
                    a.insert(from);
                }
            }

            void get_required_owner_authorities(flat_set<account_name_type> &a) const {
                if (amount.symbol == VESTS_SYMBOL) {
                    a.insert(from);
                }
            }
        };

        /**
         *  This operation converts STEEM into VFS (Vesting Fund Shares) at
         *  the current exchange rate. With this operation it is possible to
         *  give another account vesting shares so that faucets can
         *  pre-fund new accounts with vesting shares.
         */
        struct transfer_to_vesting_operation : public base_operation {
            account_name_type from;
            account_name_type to; ///< if null, then same as from
            asset amount; ///< must be STEEM

            void validate() const;

            void get_required_active_authorities(flat_set<account_name_type> &a) const {
                a.insert(from);
            }
        };

        struct transfer_to_savings_operation : public base_operation {
            account_name_type from;
            account_name_type to;
            asset amount;
            string memo;

            void get_required_active_authorities(flat_set<account_name_type> &a) const {
                a.insert(from);
            }

            void validate() const;
        };


        struct transfer_from_savings_operation : public base_operation {
            account_name_type from;
            integral_id_type request_id = 0;
            account_name_type to;
            asset amount;
            string memo;

            void get_required_active_authorities(flat_set<account_name_type> &a) const {
                a.insert(from);
            }

            void validate() const;
        };


        struct cancel_transfer_from_savings_operation : public base_operation {
            account_name_type from;
            integral_id_type request_id = 0;

            void get_required_active_authorities(flat_set<account_name_type> &a) const {
                a.insert(from);
            }

            void validate() const;
        };

        /**
         *  @class override_transfer_operation
         *  @brief Allows the issuer of an asset to transfer an asset from any account to any account if they have ove     rride_authority
         *  @ingroup operations
         *
         *  @pre amount.asset_id->issuer == issuer
         *  @pre issuer != from  because this is pointless, use a normal transfer operation
         */
        struct override_transfer_operation : public base_operation {
            struct fee_parameters_type {
                uint64_t fee = 20 * STEEMIT_BLOCKCHAIN_PRECISION;
                uint32_t price_per_kbyte = 10; /// only required for large memos.
            };

            asset fee;
            account_name_type issuer;
            /// Account to transfer asset from
            account_name_type from;
            /// Account to transfer asset to
            account_name_type to;
            /// The amount of asset to transfer from @ref from to @ref to
            asset amount;

            /// User provided data encrypted to the memo key of the "to" account
            string memo;
            extensions_type extensions;

            account_name_type fee_payer() const {
                return issuer;
            }

            void validate() const;

            share_type calculate_fee(const fee_parameters_type &k) const;
        };
    }
}

FC_REFLECT(steemit::protocol::transfer_operation, (from)(to)(amount)(memo))
FC_REFLECT(steemit::protocol::transfer_to_vesting_operation, (from)(to)(amount))

FC_REFLECT(steemit::protocol::transfer_to_savings_operation, (from)(to)(amount)(memo))
FC_REFLECT(steemit::protocol::transfer_from_savings_operation, (from)(request_id)(to)(amount)(memo))
FC_REFLECT(steemit::protocol::cancel_transfer_from_savings_operation, (from)(request_id))

FC_REFLECT(steemit::protocol::override_transfer_operation::fee_parameters_type, (fee)(price_per_kbyte))
FC_REFLECT(steemit::protocol::override_transfer_operation, (fee)(issuer)(from)(to)(amount)(memo)(extensions))

#endif //GOLOS_TRANSFER_OPERATIONS_HPP