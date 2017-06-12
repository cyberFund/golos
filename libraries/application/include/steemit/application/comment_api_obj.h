#ifndef GOLOS_COMMENT_API_OBJ_H
#define GOLOS_COMMENT_API_OBJ_H

#include <steemit/chain/account_object.hpp>
#include <steemit/chain/block_summary_object.hpp>
#include <steemit/chain/comment_object.hpp>
#include <steemit/chain/global_property_object.hpp>
#include <steemit/chain/history_object.hpp>
#include <steemit/chain/steem_objects.hpp>
#include <steemit/chain/transaction_object.hpp>
#include <steemit/chain/witness_objects.hpp>
#include <steemit/protocol/steem_operations.hpp>


namespace steemit {
    namespace application {


    using namespace steemit::chain;

    typedef chain::change_recovery_account_request_object change_recovery_account_request_api_obj;
    typedef chain::block_summary_object block_summary_api_obj;
    typedef chain::comment_vote_object comment_vote_api_obj;
    typedef chain::dynamic_global_property_object dynamic_global_property_api_obj;
    typedef chain::convert_request_object convert_request_api_obj;
    typedef chain::escrow_object escrow_api_obj;
    typedef chain::liquidity_reward_balance_object liquidity_reward_balance_api_obj;
    typedef chain::limit_order_object limit_order_api_obj;
    typedef chain::withdraw_vesting_route_object withdraw_vesting_route_api_obj;
    typedef chain::decline_voting_rights_request_object decline_voting_rights_request_api_obj;
    typedef chain::witness_vote_object witness_vote_api_obj;
    typedef chain::witness_schedule_object witness_schedule_api_obj;
    typedef chain::account_bandwidth_object account_bandwidth_api_obj;
    typedef chain::vesting_delegation_object vesting_delegation_api_obj;
    typedef chain::vesting_delegation_expiration_object vesting_delegation_expiration_api_obj;
    typedef chain::reward_fund_object reward_fund_api_obj;

struct comment_api_obj {
            comment_api_obj(const chain::comment_object &o) :
                    id(o.id),
                    category(to_string(o.category)),
                    languages(to_string(o.languages)),
                    parent_author(o.parent_author),
                    parent_permlink(to_string(o.parent_permlink)),
                    author(o.author),
                    permlink(to_string(o.permlink)),
                    title(to_string(o.title)),
                    body(to_string(o.body)),
                    json_metadata(to_string(o.json_metadata)),
                    last_update(o.last_update),
                    created(o.created),
                    active(o.active),
                    last_payout(o.last_payout),
                    depth(o.depth),
                    children(o.children),
                    children_rshares2(o.children_rshares2),
                    net_rshares(o.net_rshares),
                    abs_rshares(o.abs_rshares),
                    vote_rshares(o.vote_rshares),
                    children_abs_rshares(o.children_abs_rshares),
                    cashout_time(o.cashout_time),
                    max_cashout_time(o.max_cashout_time),
                    total_vote_weight(o.total_vote_weight),
                    reward_weight(o.reward_weight),
                    total_payout_value(o.total_payout_value),
                    curator_payout_value(o.curator_payout_value),
                    author_rewards(o.author_rewards),
                    net_votes(o.net_votes),
                    root_comment(o.root_comment),
                    max_accepted_payout(o.max_accepted_payout),
                    percent_steem_dollars(o.percent_steem_dollars),
                    allow_replies(o.allow_replies),
                    allow_votes(o.allow_votes),
                    allow_curation_rewards(o.allow_curation_rewards) {
                for (auto &route : o.beneficiaries) {
                    beneficiaries.push_back(route);
                }
            }

            bool operator< (const comment_api_obj &right) const {
                return id < right.id;
            }

            comment_api_obj() {
            }

            comment_id_type id;
            string category;
            string languages;
            account_name_type parent_author;
            string parent_permlink;
            account_name_type author;
            string permlink;

            string title;
            string body;
            string json_metadata;
            time_point_sec last_update;
            time_point_sec created;
            time_point_sec active;
            time_point_sec last_payout;

            uint8_t depth;
            uint32_t children;

            uint128_t children_rshares2;

            share_type net_rshares;
            share_type abs_rshares;
            share_type vote_rshares;

            share_type children_abs_rshares;
            time_point_sec cashout_time;
            time_point_sec max_cashout_time;
            uint64_t total_vote_weight;

            uint16_t reward_weight;

            asset total_payout_value;
            asset curator_payout_value;

            share_type author_rewards;

            int32_t net_votes;

            comment_id_type root_comment;

            asset max_accepted_payout;
            uint16_t percent_steem_dollars;
            bool allow_replies;
            bool allow_votes;
            bool allow_curation_rewards;

            vector<protocol::beneficiary_route_type> beneficiaries;
        };

     }}
        FC_REFLECT(steemit::application::comment_api_obj,
        (id)(author)(permlink)
                (languages)(category)(parent_author)(parent_permlink)
                (title)(body)(json_metadata)(last_update)(created)(active)(last_payout)
                (depth)(children)(children_rshares2)
                (net_rshares)(abs_rshares)(vote_rshares)
                (children_abs_rshares)(cashout_time)(max_cashout_time)
                (total_vote_weight)(reward_weight)(total_payout_value)(curator_payout_value)(author_rewards)(net_votes)(root_comment)
                (max_accepted_payout)(percent_steem_dollars)(allow_replies)(allow_votes)(allow_curation_rewards)
                (beneficiaries)
)
#endif //GOLOS_COMMENT_API_OBJ_H
