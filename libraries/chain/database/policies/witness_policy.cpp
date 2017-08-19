#include <steemit/chain/database/policies/witness_policy.hpp>
#include <steemit/chain/database/database_basic.hpp>
#include <steemit/chain/chain_objects/steem_objects.hpp>
#include <fc/time.hpp>
#include <steemit/chain/database/big_helper.hpp>

namespace steemit {
    namespace chain {
        namespace {
            const witness_schedule_object &get_witness_schedule_object(database_basic &db) {
                try {
                    return db.get<witness_schedule_object>();
                } FC_CAPTURE_AND_RETHROW()
            }
        }

        witness_policy::witness_policy(database_basic &ref, int) : generic_policy(ref) {
        }

        uint32_t witness_policy::witness_participation_rate() const {
            const dynamic_global_property_object &dpo = references.get_dynamic_global_properties();
            return uint64_t(STEEMIT_100_PERCENT) * dpo.recent_slots_filled.popcount() / 128;
        }

        void witness_policy::retally_witness_votes() {
            const auto &witness_idx = references.get_index<witness_index>().indices();

            // Clear all witness votes
            for (auto itr = witness_idx.begin(); itr != witness_idx.end(); ++itr) {
                references.modify(*itr, [&](witness_object &w) {
                    w.votes = 0;
                    w.virtual_position = 0;
                });
            }

            const auto &account_idx = references.get_index<account_index>().indices();

            // Apply all existing votes by account
            for (auto itr = account_idx.begin(); itr != account_idx.end(); ++itr) {
                if (itr->proxy != STEEMIT_PROXY_TO_SELF_ACCOUNT) {
                    continue;
                }

                const auto &a = *itr;

                const auto &vidx = references.get_index<witness_vote_index>().indices().get<by_account_witness>();
                auto wit_itr = vidx.lower_bound(boost::make_tuple(a.id, witness_id_type()));
                while (wit_itr != vidx.end() && wit_itr->account == a.id) {
                    database_helper::big_helper::adjust_witness_vote(references,references.get(wit_itr->witness), a.witness_vote_weight());
                    ++wit_itr;
                }
            }
        }

        void witness_policy::retally_witness_vote_counts(bool force) {
            const auto &account_idx = references.get_index<account_index>().indices();

            // Check all existing votes by account
            for (auto itr = account_idx.begin(); itr != account_idx.end(); ++itr) {
                const auto &a = *itr;
                uint16_t witnesses_voted_for = 0;
                if (force || (a.proxy != STEEMIT_PROXY_TO_SELF_ACCOUNT)) {
                    const auto &vidx = references.get_index<witness_vote_index>().indices().get<by_account_witness>();
                    auto wit_itr = vidx.lower_bound(boost::make_tuple(a.id, witness_id_type()));
                    while (wit_itr != vidx.end() && wit_itr->account == a.id) {
                        ++witnesses_voted_for;
                        ++wit_itr;
                    }
                }
                if (a.witnesses_voted_for != witnesses_voted_for) {
                    references.modify(a, [&](account_object &account) {
                        account.witnesses_voted_for = witnesses_voted_for;
                    });
                }
            }
        }

        void witness_policy::process_decline_voting_rights() {
            const auto &request_idx = references.get_index<decline_voting_rights_request_index>().indices().get<by_effective_date>();
            auto itr = request_idx.begin();

            while (itr != request_idx.end() && itr->effective_date <= references.head_block_time()) {
                const auto &account = references.get(itr->account);

                /// remove all current votes
                std::array<share_type, STEEMIT_MAX_PROXY_RECURSION_DEPTH + 1> delta;
                delta[0] = -account.vesting_shares.amount;
                for (int i = 0; i < STEEMIT_MAX_PROXY_RECURSION_DEPTH; ++i) {
                    delta[i + 1] = -account.proxied_vsf_votes[i];
                }

                database_helper::big_helper::adjust_proxied_witness_votes(references, account,delta);

                clear_witness_votes(account);

                references.modify(references.get(itr->account), [&](account_object &a) {
                    a.can_vote = false;
                    a.proxy = STEEMIT_PROXY_TO_SELF_ACCOUNT;
                });

                references.remove(*itr);
                itr = request_idx.begin();
            }
        }

        void witness_policy::clear_witness_votes(const account_object &a) {
            const auto &vidx = references.get_index<witness_vote_index>().indices().get<by_account_witness>();
            auto itr = vidx.lower_bound(boost::make_tuple(a.id, witness_id_type()));
            while (itr != vidx.end() && itr->account == a.id) {
                const auto &current = *itr;
                ++itr;
                references.remove(current);
            }

            if (references.has_hardfork(STEEMIT_HARDFORK_0_6__104)) { // TODO: this check can be removed after hard fork
                references.modify(a, [&](account_object &acc) {
                    acc.witnesses_voted_for = 0;
                });
            }
        }

        void witness_policy::update_median_witness_props() {
            const witness_schedule_object &wso = get_witness_schedule_object(references);

            /// fetch all witness objects
            vector<const witness_object *> active;
            active.reserve(wso.num_scheduled_witnesses);
            for (int i = 0; i < wso.num_scheduled_witnesses; i++) {
                active.push_back(&get_witness(wso.current_shuffled_witnesses[i]));
            }

            /// sort them by account_creation_fee
            std::sort(active.begin(), active.end(), [&](const witness_object *a, const witness_object *b) {
                return a->props.account_creation_fee.amount < b->props.account_creation_fee.amount;
            });
            asset median_account_creation_fee = active[active.size() / 2]->props.account_creation_fee;

            /// sort them by maximum_block_size
            std::sort(active.begin(), active.end(), [&](const witness_object *a, const witness_object *b) {
                return a->props.maximum_block_size < b->props.maximum_block_size;
            });
            uint32_t median_maximum_block_size = active[active.size() / 2]->props.maximum_block_size;

            /// sort them by sbd_interest_rate
            std::sort(active.begin(), active.end(), [&](const witness_object *a, const witness_object *b) {
                return a->props.sbd_interest_rate < b->props.sbd_interest_rate;
            });
            uint16_t median_sbd_interest_rate = active[active.size() /
                                                       2]->props.sbd_interest_rate;

            references.modify(wso, [&](witness_schedule_object &_wso) {
                _wso.median_props.account_creation_fee = median_account_creation_fee;
                _wso.median_props.maximum_block_size = median_maximum_block_size;
                _wso.median_props.sbd_interest_rate = median_sbd_interest_rate;
            });

            references.modify(references.get_dynamic_global_properties(), [&](dynamic_global_property_object &_dgpo) {
                _dgpo.maximum_block_size = median_maximum_block_size;
                _dgpo.sbd_interest_rate = median_sbd_interest_rate;
            });
        }

        void witness_policy::update_signing_witness(const witness_object &signing_witness,
                                                    const protocol::signed_block &new_block) {
            try {
                const dynamic_global_property_object &dpo = references.get_dynamic_global_properties();
                uint64_t new_block_aslot = dpo.current_aslot + references.get_slot_at_time(new_block.timestamp);

                references.modify(signing_witness, [&](witness_object &_wit) {
                    _wit.last_aslot = new_block_aslot;
                    _wit.last_confirmed_block_num = new_block.block_num();
                });
            } FC_CAPTURE_AND_RETHROW()
        }

        void witness_policy::reset_virtual_schedule_time() {
            const witness_schedule_object &wso = get_witness_schedule_object(references);
            references.modify(wso, [&](witness_schedule_object &o) {
                o.current_virtual_time = fc::uint128(); // reset it 0
            });

            const auto &idx = references.get_index<witness_index>().indices();
            for (const auto &witness : idx) {
                references.modify(witness, [&](witness_object &wobj) {
                    wobj.virtual_position = fc::uint128();
                    wobj.virtual_last_update = wso.current_virtual_time;
                    wobj.virtual_scheduled_time = VIRTUAL_SCHEDULE_LAP_LENGTH2 / (wobj.votes.value + 1);
                });
            }
        }

        steemit::protocol::asset witness_policy::create_vesting(const account_object &to_account, asset steem) {
            return database_helper::big_helper::create_vesting(references, to_account, steem);
        }

        void witness_policy::adjust_witness_votes(const account_object &a, share_type delta) {
            database_helper::big_helper::adjust_witness_votes(references, a, delta);
        }

        void witness_policy::adjust_witness_vote(const witness_object &witness, share_type delta) {
            database_helper::big_helper::adjust_witness_vote(references, witness, delta);
        }

        void witness_policy::adjust_proxied_witness_votes(const account_object &a, share_type delta, int depth) {
            database_helper::big_helper::adjust_proxied_witness_votes(references, a, delta, depth);
        }

        void witness_policy::adjust_proxied_witness_votes(const account_object &a, const std::array<share_type, STEEMIT_MAX_PROXY_RECURSION_DEPTH + 1> &delta, int depth) {
            database_helper::big_helper::adjust_proxied_witness_votes(references, a, delta, depth);
        }

        const witness_object &witness_policy::get_witness(const account_name_type &name) const {
            try {
                return references.get<witness_object, by_name>(name);
            } FC_CAPTURE_AND_RETHROW((name))
        }

        const witness_object *witness_policy::find_witness(const account_name_type &name) const {
            return references.find<witness_object, by_name>(name);
        }
    }
}
