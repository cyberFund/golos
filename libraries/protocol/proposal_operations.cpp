#include <steemit/protocol/proposal_operations.hpp>
#include <steemit/protocol/operations.hpp>
#include <steemit/protocol/types.hpp>

#include <fc/smart_ref_impl.hpp>
#include <fc/exception/exception.hpp>
#include <fc/time.hpp>

namespace steemit {
    namespace protocol {
        void proposal_create_operation::validate() const {
            FC_ASSERT(!proposed_ops.empty());
            for (const auto &op : proposed_ops) {
                operation_validate(op.op);
            }
        }


        void proposal_update_operation::validate() const {
            FC_ASSERT(!(active_approvals_to_add.empty() &&
                        active_approvals_to_remove.empty() &&
                        owner_approvals_to_add.empty() &&
                        owner_approvals_to_remove.empty() &&
                        key_approvals_to_add.empty() &&
                        key_approvals_to_remove.empty()));
            for (auto a : active_approvals_to_add) {
                FC_ASSERT(active_approvals_to_remove.find(a) ==
                          active_approvals_to_remove.end(),
                        "Cannot add and remove approval at the same time.");
            }
            for (auto a : owner_approvals_to_add) {
                FC_ASSERT(owner_approvals_to_remove.find(a) ==
                          owner_approvals_to_remove.end(),
                        "Cannot add and remove approval at the same time.");
            }
            for (auto a : key_approvals_to_add) {
                FC_ASSERT(key_approvals_to_remove.find(a) ==
                          key_approvals_to_remove.end(),
                        "Cannot add and remove approval at the same time.");
            }
        }

        void proposal_delete_operation::validate() const {

        }

        void proposal_update_operation::get_required_authorities(vector<authority> &o) const {
            authority auth;
            for (const auto &k : key_approvals_to_add) {
                auth.key_auths[k] = 1;
            }
            for (const auto &k : key_approvals_to_remove) {
                auth.key_auths[k] = 1;
            }
            auth.weight_threshold = auth.key_auths.size();

            o.emplace_back(std::move(auth));
        }

        void proposal_update_operation::get_required_active_authorities(flat_set<account_name_type> &a) const {
            for (const auto &i : active_approvals_to_add) {
                a.insert(i);
            }
            for (const auto &i : active_approvals_to_remove) {
                a.insert(i);
            }
        }

        void proposal_update_operation::get_required_owner_authorities(flat_set<account_name_type> &a) const {
            for (const auto &i : owner_approvals_to_add) {
                a.insert(i);
            }
            for (const auto &i : owner_approvals_to_remove) {
                a.insert(i);
            }
        }
    }
} // graphene::chain
