#pragma once

#include <steemit/protocol/base.hpp>
#include <steemit/protocol/chain_properties.hpp>

namespace steemit {
    namespace protocol {

        /**
         * @brief Create a committee_member object, as a bid to hold a committee_member seat on the network.
         * @ingroup operations
         *
         * Accounts which wish to become committee_members may use this operation to create a committee_member object which stakeholders may
         * vote on to approve its position as a committee_member.
         */
        struct committee_member_create_operation : public base_operation {
            /// The account which owns the committee_member. This account pays the fee for this operation.
            account_name_type committee_member_account;
            std::string url;

            void validate() const;
        };

        /**
         * @brief Update a committee_member object.
         * @ingroup operations
         *
         * Currently the only field which can be updated is the `url`
         * field.
         */
        struct committee_member_update_operation : public base_operation {
            /// The account which owns the committee_member. This account pays the fee for this operation.
            account_name_type committee_member_account;
            optional<std::string> new_url;

            void validate() const;
        };

        /**
         * @brief Used by committee_members to update the global parameters of the blockchain.
         * @ingroup operations
         *
         * This operation allows the committee_members to update the global parameters on the blockchain. These control various
         * tunable aspects of the chain, including block and maintenance intervals, maximum data sizes, the fees charged by
         * the network, etc.
         *
         * This operation may only be used in a proposed transaction, and a proposed transaction which contains this
         * operation must have a review period specified in the current global parameters before it may be accepted.
         */
        struct committee_member_update_global_parameters_operation
                : public base_operation {
            chain_properties<2> new_parameters;

            void validate() const;
        };

        /// TODO: committee_member_resign_operation : public base_operation
    }
} // graphene::chain
FC_REFLECT(steemit::protocol::committee_member_create_operation,
        (committee_member_account)(url))
FC_REFLECT(steemit::protocol::committee_member_update_operation,
        (committee_member_account)(new_url))
FC_REFLECT(steemit::protocol::committee_member_update_global_parameters_operation, (new_parameters));
