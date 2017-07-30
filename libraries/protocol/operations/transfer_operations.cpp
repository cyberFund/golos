#include <steemit/protocol/operations/transfer_operations.hpp>

#include <fc/io/json.hpp>
#include <fc/utf8.hpp>

namespace steemit {
    namespace protocol {
        /// TODO: after the hardfork, we can rename this method validate_permlink because it is strictily less restrictive than before
        ///  Issue #56 contains the justificiation for allowing any UTF-8 string to serve as a permlink, content will be grouped by tags
        ///  going forward.
        inline void validate_permlink(const string &permlink) {
            FC_ASSERT(permlink.size() <
                      STEEMIT_MAX_PERMLINK_LENGTH, "permlink is too long");
            FC_ASSERT(fc::is_utf8(permlink), "permlink not formatted in UTF8");
        }

        inline void validate_account_name(const string &name) {
            FC_ASSERT(is_valid_account_name(name), "Account name ${n} is invalid", ("n", name));
        }

        bool inline is_asset_type(asset asset, asset_symbol_type symbol) {
            return asset.symbol == symbol;
        }

        void transfer_operation::validate() const {
            try {
                validate_account_name(from);
                validate_account_name(to);
                FC_ASSERT(amount.symbol !=
                          VESTS_SYMBOL, "transferring of Golos Power (STMP) is not allowed.");
                FC_ASSERT(amount.amount >
                          0, "Cannot transfer a negative amount (aka: stealing)");
                FC_ASSERT(memo.size() <
                          STEEMIT_MAX_MEMO_SIZE, "Memo is too large");
                FC_ASSERT(fc::is_utf8(memo), "Memo is not UTF8");
            } FC_CAPTURE_AND_RETHROW((*this))
        }

        void transfer_to_vesting_operation::validate() const {
            validate_account_name(from);
            FC_ASSERT(is_asset_type(amount, STEEM_SYMBOL), "Amount must be STEEM");
            if (to != account_name_type()) {
                validate_account_name(to);
            }
            FC_ASSERT(amount >
                      asset(0, STEEM_SYMBOL), "Must transfer a nonzero amount");
        }

        void transfer_to_savings_operation::validate() const {
            validate_account_name(from);
            validate_account_name(to);
            FC_ASSERT(amount.amount > 0);
            FC_ASSERT(amount.symbol == STEEM_SYMBOL ||
                      amount.symbol == SBD_SYMBOL);
            FC_ASSERT(memo.size() < STEEMIT_MAX_MEMO_SIZE, "Memo is too large");
            FC_ASSERT(fc::is_utf8(memo), "Memo is not UTF8");
        }

        void transfer_from_savings_operation::validate() const {
            validate_account_name(from);
            validate_account_name(to);
            FC_ASSERT(amount.amount > 0);
            FC_ASSERT(amount.symbol == STEEM_SYMBOL ||
                      amount.symbol == SBD_SYMBOL);
            FC_ASSERT(memo.size() < STEEMIT_MAX_MEMO_SIZE, "Memo is too large");
            FC_ASSERT(fc::is_utf8(memo), "Memo is not UTF8");
        }

        void cancel_transfer_from_savings_operation::validate() const {
            validate_account_name(from);
        }


        void override_transfer_operation::validate() const {
            FC_ASSERT(from != to);
            FC_ASSERT(amount.amount > 0);
            FC_ASSERT(issuer != from);
        }
    }
}