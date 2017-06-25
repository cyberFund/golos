#include <steemit/chain/committee_member_evaluator.hpp>
#include <steemit/chain/database.hpp>
#include <steemit/chain/account_object.hpp>

#include <fc/smart_ref_impl.hpp>

namespace steemit {
    namespace chain {
        void committee_member_create_evaluator::do_apply(const committee_member_create_operation &op) {
            try {
                vote_id_type vote_id;
                _db.modify(_db.get_global_properties(), [&vote_id](global_property_object<1> &p) {
                    vote_id = p.get_next_vote_id(vote_id_type::committee);
                });

                _db.create<committee_member_object>([&](committee_member_object &obj) {
                    obj.committee_member_account = op.committee_member_account;
                    obj.vote_id = vote_id;
                    obj.url = op.url;
                });
            } FC_CAPTURE_AND_RETHROW((op))
        }

        void committee_member_update_evaluator::do_apply(const committee_member_update_operation &op) {
            FC_ASSERT(db().get_committee_member(op.committee_member_account).committee_member_account == op.committee_member_account);

            try {
                _db.modify(_db.get_committee_member(op.committee_member_account), [&](committee_member_object &com) {
                    if (op.new_url.valid()) {
                        com.url = *op.new_url;
                    }
                });
            } FC_CAPTURE_AND_RETHROW((op))
        }

        void committee_member_update_global_parameters_evaluator::do_apply(const committee_member_update_global_parameters_operation &o) {
            try {
                db().modify(db().get_global_properties(), [&o](global_property_object<1> &p) {
                    p.pending_parameters = o.new_parameters;
                });
            } FC_CAPTURE_AND_RETHROW((o))
        }
    }
}