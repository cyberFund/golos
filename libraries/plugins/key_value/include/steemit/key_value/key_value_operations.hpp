#ifndef GOLOS_KEY_VALUE_OPERATIONS_H
#define GOLOS_KEY_VALUE_OPERATIONS_H

#include <steemit/protocol/base.hpp>

#include <steemit/key_value/key_value_plugin.hpp>

namespace steemit {
    namespace key_value {
        using steemit::protocol::base_operation;

        struct create_first_key_value_operation : base_operation {
            account_name_type owner;
            string system;
            fc::uint128_t block_number;
            string block_hash;
            string ipfs_hash_link;
            fc::time_point_sec block_timestamp;
            fc::time_point_sec timestamp;

            void validate() const;

            void get_required_posting_authorities(flat_set<protocol::account_name_type> &a) const {
                a.insert(owner);
            }
        };

        struct update_first_key_value_operation : base_operation {
            account_name_type owner;
            string system;
            fc::uint128_t block_number;
            string block_hash;
            string ipfs_hash_link;
            fc::time_point_sec block_timestamp;
            fc::time_point_sec timestamp;

            void validate() const;

            void get_required_posting_authorities(flat_set<protocol::account_name_type> &a) const {
                a.insert(owner);
            }
        };

        struct delete_first_key_value_operation : base_operation {
            account_name_type owner;
            string system;
            fc::uint128_t block_number;
            string block_hash;
            string ipfs_hash_link;
            fc::time_point_sec block_timestamp;
            fc::time_point_sec timestamp;

            void validate() const;

            void get_required_posting_authorities(flat_set<protocol::account_name_type> &a) const {
                a.insert(owner);
            }
        };

        typedef fc::static_variant<create_first_key_value_operation, update_first_key_value_operation, delete_first_key_value_operation> key_value_plugin_operation;

        STEEMIT_DEFINE_PLUGIN_EVALUATOR(key_value_plugin, key_value_plugin_operation, create_first_key_value);

        STEEMIT_DEFINE_PLUGIN_EVALUATOR(key_value_plugin, key_value_plugin_operation, update_first_key_value);

        STEEMIT_DEFINE_PLUGIN_EVALUATOR(key_value_plugin, key_value_plugin_operation, delete_first_key_value);
    }
}

FC_REFLECT(steemit::key_value::create_first_key_value_operation, (owner)(system)(block_hash)(block_number)(ipfs_hash_link)(block_timestamp)(timestamp))
FC_REFLECT(steemit::key_value::update_first_key_value_operation, (owner)(system)(block_hash)(block_number)(ipfs_hash_link)(block_timestamp)(timestamp))
FC_REFLECT(steemit::key_value::delete_first_key_value_operation, (owner)(system)(block_hash)(block_number)(ipfs_hash_link)(block_timestamp)(timestamp))

STEEMIT_DECLARE_OPERATION_TYPE(steemit::key_value::key_value_plugin_operation)

FC_REFLECT_TYPENAME(steemit::key_value::key_value_plugin_operation)

#endif //GOLOS_KEY_VALUE_OPERATIONS_H