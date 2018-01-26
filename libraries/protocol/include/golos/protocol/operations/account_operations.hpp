#ifndef GOLOS_ACCOUNT_OPERATIONS_HPP
#define GOLOS_ACCOUNT_OPERATIONS_HPP

#include <golos/protocol/asset.hpp>
#include <golos/protocol/base.hpp>
#include <golos/protocol/block_header.hpp>

namespace golos {
    namespace protocol {
        /**
         *  @ingroup operations
         *  @brief Creates new account
         *
         *  @param fee Paid by creator
         *  @param creator Account which pays the fee and is allowed to perform an account key reset operation
         *  @param new_account_name Created account name
         *  @param owner Created account public owner key
         *  @param active Created account public active key
         *  @param posting Created account public posting key
         *  @param memo_key The memo key is the key this account will typically use to encrypt/sign transaction memos and other non-validated account activities. This field is here to prevent confusion if the active authority has zero or multiple keys in it.
         *  @param json_metadata Exact content of this member will be available in account_object::json_metadata
         *
         *  @tparam Major Indicates the major protocol version this operation will be used for
         *  @tparam Hardfork Indicates the hardfork version this operation will be used for
         *  @tparam Release Indicates the release protocol version this operation will be used for
         *
         *  @note In fact business logic protocol versioninig scheme is not bounded in any way to business logic chain versioning, but it was decided to make them coincide
         *
         *  This particular operation is a data context for account_create_evaluator<Major, Hardfork, Release>. The result of a particular operation is an account_object created with the params set from the operation
         */
        template<uint8_t Major, uint8_t Hardfork, uint16_t Release>
        struct account_create_operation : public base_operation<Major, Hardfork, Release> {
            asset<Major, Hardfork, Release> fee;
            account_name_type creator;
            account_name_type new_account_name;
            authority owner;
            authority active;
            authority posting;

            public_key_type memo_key;
            std::string json_metadata;

            void validate() const;

            void get_required_active_authorities(flat_set<account_name_type> &a) const {
                a.insert(creator);
            }
        };

        /**
         *  @ingroup operations
         *  @brief Creates account with initially delegated Steem Power
         *
         *  @param fee Paid by creator
         *  @param delegation Initial vesting delegation amount
         *  @param creator Account which pays the fee and is allowed to perform an account key reset operation
         *  @param new_account_name Created account name
         *  @param owner Created account public owner key
         *  @param active Created account public active key
         *  @param posting Created account public posting key
         *  @param memo_key The memo key is the key this account will typically use to encrypt/sign transaction memos and other non-validated account activities. This field is here to prevent confusion if the active authority has zero or multiple keys in it.
         *  @param json_metadata Exact content of this member will be available in account_object::json_metadata
         *
         *  @tparam Major Indicates the major protocol version this operation will be used for
         *  @tparam Hardfork Indicates the hardfork version this operation will be used for
         *  @tparam Release Indicates the release protocol version this operation will be used for
         *
         *  @note In fact business logic protocol versioninig scheme is not bounded in any way to business logic chain versioning, but it was decided to make them coincide
         *
         *  This particular operation is a data context for account_create_with_delegation_evaluator<Major, Hardfork, Release>. The result of a particular operation is an account_object created with the params set from the operation
         */
        template<uint8_t Major, uint8_t Hardfork, uint16_t Release>
        struct account_create_with_delegation_operation : public base_operation<Major, Hardfork, Release> {
            asset<Major, Hardfork, Release> fee;
            asset<Major, Hardfork, Release> delegation;
            account_name_type creator;
            account_name_type new_account_name;
            authority owner;
            authority active;
            authority posting;

            public_key_type memo_key;
            std::string json_metadata;

            extensions_type extensions;

            void validate() const;

            void get_required_active_authorities(flat_set<account_name_type> &a) const {
                a.insert(creator);
            }
        };

        /**
         * @ingroup operations
         * @brief Update an existing account
         *
         * @param account Account to update
         * @param owner Account public owner key
         * @param active Account public active key
         * @param posting Account public posting key
         * @param memo_key The memo key is the key this account will typically use to encrypt/sign transaction memos and other non-validated account activities. This field is here to prevent confusion if the active authority has zero or multiple keys in it.
         * @param json_metadata
         *
         * This operation is used to update an existing account. It can be used to update the authorities, or adjust the opt ions on the account.
         */
        template<uint8_t Major, uint8_t Hardfork, uint16_t Release>
        struct account_update_operation : public base_operation<Major, Hardfork, Release> {
            account_name_type account;
            optional<authority> owner;
            optional<authority> active;
            optional<authority> posting;

            public_key_type memo_key;
            std::string json_metadata;

            void validate() const;

            void get_required_owner_authorities(flat_set<account_name_type> &a) const {
                if (owner) {
                    a.insert(account);
                }
            }

            void get_required_active_authorities(flat_set<account_name_type> &a) const {
                if (!owner) {
                    a.insert(account);
                }
            }
        };

        /**
         * @brief This operation is used to whitelist and blacklist accounts, primarily for transacting in whitelisted ass     ets
         * @ingroup operations
         *
         * @param fee Paid by authorizing_account
         * @param authorizing_account The account which is specifying an opinion of another account
         * @param account_to_list The account being opined about
         * @param new_listing The new white and blacklist status of account_to_list, as determined by authorizing_account. This is a bitfield using values defined in the @ref account_listing enum
         * @param extensions
         *
         * Accounts can freely specify opinions about other accounts, in the form of either whitelisting or blacklisting
         * them. This information is used in chain validation only to determine whether an account is authorized to tra     nsact
         * in an asset type which enforces a whitelist, but third parties can use this information for other uses as wel     l,
         * as long as it does not conflict with the use of whitelisted assets.
         *
         * An asset which enforces a whitelist specifies a list of accounts to maintain its whitelist, and a list of
         * accounts to maintain its blacklist. In order for a given account A to hold and transact in a whitelisted ass     et S,
         * A must be whitelisted by at least one of S's whitelist_authorities and blacklisted by none of S's
         * blacklist_authorities. If A receives a balance of S, and is later removed from the whitelist(s) which all     owed it
         * to hold S, or added to any blacklist S specifies as authoritative, A's balance of S will be frozen until A's
         * authorization is reinstated.
         *
         * This operation requires authorizing_account's signature, but not account_to_list's. The fee is paid by
         * authorizing_account.
         */
        template<uint8_t Major, uint8_t Hardfork, uint16_t Release>
        struct account_whitelist_operation : public base_operation<Major, Hardfork, Release> {
            enum account_listing {
                no_listing = 0x0, ///< No opinion is specified about this account
                white_listed = 0x1, ///< This account is whitelisted, but not blacklisted
                black_listed = 0x2, ///< This account is blacklisted, but not whitelisted
                white_and_black_listed =
                white_listed | black_listed ///< This account is both whitelisted and blacklisted
            };

            /// Paid by authorizing_account
            asset<Major, Hardfork, Release> fee;
            /// The account which is specifying an opinion of another account
            account_name_type authorizing_account;
            /// The account being opined about
            account_name_type account_to_list;
            /// The new white and blacklist status of account_to_list, as determined by authorizing_account
            /// This is a bitfield using values defined in the account_listing enum
            uint8_t new_listing = no_listing;
            extensions_type extensions;

            void validate() const;
        };
    }
}

FC_REFLECT((golos::protocol::account_create_operation<0, 16, 0>),
           (fee)(creator)(new_account_name)(owner)(active)(posting)(memo_key)(json_metadata))
FC_REFLECT((golos::protocol::account_create_operation<0, 17, 0>),
           (fee)(creator)(new_account_name)(owner)(active)(posting)(memo_key)(json_metadata))

FC_REFLECT((golos::protocol::account_create_with_delegation_operation<0, 17, 0>),
           (fee)(delegation)(creator)(new_account_name)(owner)(active)(posting)(memo_key)(json_metadata)(extensions))

FC_REFLECT((golos::protocol::account_update_operation<0, 16, 0>),
           (account)(owner)(active)(posting)(memo_key)(json_metadata))
FC_REFLECT((golos::protocol::account_update_operation<0, 17, 0>),
           (account)(owner)(active)(posting)(memo_key)(json_metadata))

FC_REFLECT_ENUM(BOOST_IDENTITY_TYPE((golos::protocol::account_whitelist_operation<0, 16, 0>))
                        ::account_listing, (no_listing)(white_listed)(black_listed)(white_and_black_listed));
FC_REFLECT_ENUM(BOOST_IDENTITY_TYPE((golos::protocol::account_whitelist_operation<0, 17, 0>))
                        ::account_listing, (no_listing)(white_listed)(black_listed)(white_and_black_listed));

FC_REFLECT((golos::protocol::account_whitelist_operation<0, 16, 0>),
           (fee)(authorizing_account)(account_to_list)(new_listing)(extensions))
FC_REFLECT((golos::protocol::account_whitelist_operation<0, 17, 0>),
           (fee)(authorizing_account)(account_to_list)(new_listing)(extensions))

#endif //GOLOS_ACCOUNT_OPERATIONS_HPP