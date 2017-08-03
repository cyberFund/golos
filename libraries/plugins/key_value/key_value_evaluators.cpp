#include <steemit/key_value/key_value_operations.hpp>

namespace steemit {
    namespace key_value {
        void create_first_key_value_evaluator::do_apply(const create_first_key_value_operation &o) {
            FC_ASSERT(db.find_account(o.owner));

            db.create<first_key_value_object>([&](first_key_value_object &c) {
                c.owner = o.owner;
                c.block_hash = o.block_hash;
                c.block_number = o.block_number;
                c.system = o.system;
                c.ipfs_hash_link = o.ipfs_hash_link;
                c.block_timestamp = o.block_timestamp;
                c.timestamp = o.timestamp;
            });
        }

        void update_first_key_value_evaluator::do_apply(const update_first_key_value_operation &o) {
            FC_ASSERT(db.find_account(o.owner));
        }

        void delete_first_key_value_evaluator::do_apply(const delete_first_key_value_operation &o) {
            FC_ASSERT(db.find_account(o.owner));
        }
    }
}
