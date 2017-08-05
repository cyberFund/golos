#ifndef GOLOS_WITNESS_POLICY_HPP
#define GOLOS_WITNESS_POLICY_HPP

#include <algorithm>

#include "steemit/chain/database/generic_policy.hpp"
#include "steemit/chain/dynamic_extension/worker.hpp"
#include <steemit/protocol/block.hpp>
#include <steemit/chain/steem_objects.hpp>

namespace steemit {
namespace chain {
struct witness_policy : public generic_policy {

    witness_policy(const witness_policy &) = default;

    witness_policy &operator=(const witness_policy &) = default;

    witness_policy(witness_policy &&) = default;

    witness_policy &operator=(witness_policy &&) = default;

    virtual ~witness_policy() = default;

    witness_policy(database_basic &ref,int);

    uint32_t witness_participation_rate() const;

    void retally_witness_votes();


    void retally_witness_vote_counts(bool force);

    void process_decline_voting_rights();


    void clear_witness_votes(const account_object &a);

/*
    void adjust_witness_votes(const account_object &a, share_type delta) {
        const auto &vidx = references.get_index<witness_vote_index>().indices().get<by_account_witness>();
        auto itr = vidx.lower_bound(boost::make_tuple(a.id, witness_id_type()));
        while (itr != vidx.end() && itr->account == a.id) {
            adjust_witness_vote(get(itr->witness), delta);
            ++itr;
        }
    }
*/

    void update_median_witness_props();

    void update_signing_witness(const witness_object &signing_witness, const protocol::signed_block &new_block);

    void reset_virtual_schedule_time();

    asset create_vesting(const account_object &to_account, asset steem);

    void adjust_witness_votes(const account_object &a, share_type delta);

    void adjust_witness_vote(const witness_object &witness, share_type delta);

    void adjust_proxied_witness_votes(const account_object &a, share_type delta, int depth = 0);
    void adjust_proxied_witness_votes(const account_object &a, const std::array<share_type,STEEMIT_MAX_PROXY_RECURSION_DEPTH + 1> &delta, int depth = 0);
};
}}
#endif //GOLOS_WITNESS_POLICY_HPP
