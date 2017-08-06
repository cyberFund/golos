#ifndef GOLOS_COMMENT_OBJECT_POLICY_HPP
#define GOLOS_COMMENT_OBJECT_POLICY_HPP

#include <steemit/chain/utilities/reward.hpp>
#include "steemit/chain/database/generic_policy.hpp"

namespace steemit {
namespace chain {


struct reward_fund_context {
    uint128_t recent_rshares2 = 0;
    asset reward_balance = asset(0, STEEM_SYMBOL);
    share_type steem_awarded = 0;
};

class comment_policy : public generic_policy {
public:
    comment_policy(const comment_policy &) = default;

    comment_policy &operator=(const comment_policy &) = default;

    comment_policy(comment_policy &&) = default;

    comment_policy &operator=(comment_policy &&) = default;

    virtual ~comment_policy() = default;

    comment_policy(database_basic &ref,int);

    const time_point_sec calculate_discussion_payout_time(const comment_object &comment) const;

    //const reward_fund_object &get_reward_fund(const comment_object &c) const;


    void adjust_total_payout(const comment_object &cur, const asset &sbd_created, const asset &curator_sbd_value, const asset &beneficiary_value);

/**
 *  This method will iterate through all comment_vote_objects and give them
 *  (max_rewards * weight) / c.total_vote_weight.
 *
 *  @returns unclaimed rewards.
 */
    share_type pay_curators(const comment_object &c, share_type &max_rewards);

    void fill_comment_reward_context_local_state(utilities::comment_reward_context &ctx, const comment_object &comment);


    void update_children_rshares2(const comment_object &c, const fc::uint128_t &old_rshares2, const fc::uint128_t &new_rshares2);

    /** This method updates total_reward_shares2 on DGPO, and children_rshares2 on comments, when a comment's rshares2 changes
    * from old_rshares2 to new_rshares2.  Maintaining invariants that children_rshares2 is the sum of all descendants' rshares2,
    * and dgpo.total_reward_shares2 is the total number of rshares2 outstanding.
    */

    void adjust_rshares2(const comment_object &c, fc::uint128_t old_rshares2, fc::uint128_t new_rshares2);

    void retally_comment_children();

    void process_comment_cashout();


    share_type cashout_comment_helper(utilities::comment_reward_context &ctx, const comment_object &comment);


    time_point_sec get_payout_extension_time(const comment_object &input_comment, const asset &input_cost) const;


    uint16_t get_curation_rewards_percent(const comment_object &c) const;

    void perform_vesting_share_split(uint32_t magnitude);

    const comment_object &get_comment(const account_name_type &author, const shared_string &permlink) const;

    const comment_object *find_comment(const account_name_type &author, const shared_string &permlink) const;

    const comment_object &get_comment(const account_name_type &author, const string &permlink) const;

    const comment_object *find_comment(const account_name_type &author, const string &permlink) const;

};
}}

#endif