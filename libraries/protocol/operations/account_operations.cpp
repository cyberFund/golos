#include <steemit/protocol/operations/account_operations.hpp>

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

        void account_create_operation::validate() const {
            validate_account_name(new_account_name);
            FC_ASSERT(is_asset_type(fee, STEEM_SYMBOL), "Account creation fee must be STEEM");
            owner.validate();
            active.validate();

            if (json_metadata.size() > 0) {
                FC_ASSERT(fc::is_utf8(json_metadata), "JSON Metadata not formatted in UTF8");
                FC_ASSERT(fc::json::is_valid(json_metadata), "JSON Metadata not valid JSON");
            }
            FC_ASSERT(fee >=
                      asset(0, STEEM_SYMBOL), "Account creation fee cannot be negative");
        }

        void account_create_with_delegation_operation::validate() const {
            validate_account_name(new_account_name);
            validate_account_name(creator);
            FC_ASSERT(is_asset_type(fee, STEEM_SYMBOL), "Account creation fee must be STEEM");
            FC_ASSERT(is_asset_type(delegation, VESTS_SYMBOL), "Delegation must be VESTS");

            owner.validate();
            active.validate();
            posting.validate();

            if (json_metadata.size() > 0) {
                FC_ASSERT(fc::is_utf8(json_metadata), "JSON Metadata not formatted in UTF8");
                FC_ASSERT(fc::json::is_valid(json_metadata), "JSON Metadata not valid JSON");
            }

            FC_ASSERT(fee >=
                      asset(0, STEEM_SYMBOL), "Account creation fee cannot be negative");
            FC_ASSERT(delegation >=
                      asset(0, VESTS_SYMBOL), "Delegation cannot be negative");
        }

        void account_update_operation::validate() const {
            validate_account_name(account);
            /*if( owner )
               owner->validate();
            if( active )
               active->validate();
            if( posting )
               posting->validate();*/

            if (json_metadata.size() > 0) {
                FC_ASSERT(fc::is_utf8(json_metadata), "JSON Metadata not formatted in UTF8");
                FC_ASSERT(fc::json::is_valid(json_metadata), "JSON Metadata not valid JSON");
            }
        }
    }
}
