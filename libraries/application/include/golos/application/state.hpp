#pragma once

#include <golos/application/applied_operation.hpp>
#include <golos/application/api_objects/steem_api_objects.hpp>

#include <golos/chain/objects/global_property_object.hpp>
#include <golos/chain/objects/account_object.hpp>
#include <golos/chain/objects/steem_objects.hpp>

namespace golos {
    namespace application {
        using std::string;
        using std::vector;

        struct extended_limit_order : public limit_order_object {
            extended_limit_order() {
            }

            extended_limit_order(const limit_order_object &o) : limit_order_object(o) {
            }

            double real_price = 0;
            bool rewarded = false;
        };

        struct discussion_index {
            std::string category;    /// category by which everything is filtered
            std::vector<std::string> trending;    /// trending posts over the last 24 hours
            std::vector<std::string> payout;      /// pending posts by payout
            std::vector<std::string> payout_comments; /// pending comments by payout
            std::vector<std::string> trending30;  /// pending lifetime payout
            std::vector<std::string> created;     /// creation date
            std::vector<std::string> responses;   /// creation date
            std::vector<std::string> updated;     /// creation date
            std::vector<std::string> active;      /// last update or reply
            std::vector<std::string> votes;       /// last update or reply
            std::vector<std::string> cashout;     /// last update or reply
            std::vector<std::string> maturing;    /// about to be paid out
            std::vector<std::string> best;        /// total lifetime payout
            std::vector<std::string> hot;         /// total lifetime payout
            std::vector<std::string> promoted;    /// pending lifetime payout
        };

        struct category_index {
            std::vector<std::string> active;   /// recent activity
            std::vector<std::string> recent;   /// recently created
            std::vector<std::string> best;     /// total lifetime payout
        };

        struct tag_index {
            std::vector<std::string> trending; /// pending payouts
        };

        struct vote_state {
            std::string voter;
            uint64_t weight = 0;
            int64_t rshares = 0;
            int16_t percent = 0;
            share_type reputation = 0;
            time_point_sec time;
        };

        struct account_vote {
            std::string authorperm;
            uint64_t weight = 0;
            int64_t rshares = 0;
            int16_t percent = 0;
            time_point_sec time;
        };

        struct discussion : public comment_api_object {
            discussion(const comment_object &o) : comment_api_object(o) {
            }

            discussion() {
            }

            std::string url; /// /category/@rootauthor/root_permlink#author/permlink
            std::string root_title;
            asset<0, 17, 0> pending_payout_value = asset<0, 17, 0>(0, SBD_SYMBOL_NAME); ///< sbd
            asset<0, 17, 0> total_pending_payout_value = asset<0, 17, 0>(0, SBD_SYMBOL_NAME); ///< sbd including replies
            std::vector<vote_state> active_votes;
            std::vector<std::string> replies; ///< author/slug mapping
            share_type author_reputation = 0;
            asset<0, 17, 0> promoted = asset<0, 17, 0>(0, SBD_SYMBOL_NAME);
            uint32_t body_length = 0;
            std::vector<account_name_type> reblogged_by;
            optional<account_name_type> first_reblogged_by;
            optional<time_point_sec> first_reblogged_on;
        };

        /**
         *  Convert's vesting shares
         */
        struct extended_account : public account_api_object {
            extended_account() {
            }

            extended_account(const account_object &a, const database &db) : account_api_object(a, db) {
            }

            asset<0, 17, 0> vesting_balance; /// convert vesting_shares to vesting steem
            share_type reputation = 0;
            std::map<uint64_t, applied_operation> transfer_history; /// transfer to/from vesting
            std::map<uint64_t, applied_operation> market_history; /// limit order / cancel / fill
            std::map<uint64_t, applied_operation> post_history;
            std::map<uint64_t, applied_operation> vote_history;
            std::map<uint64_t, applied_operation> other_history;
            std::set<std::string> witness_votes;
            std::vector<std::pair<std::string, uint32_t>> tags_usage;
            std::vector<std::pair<account_name_type, uint32_t>> guest_bloggers;

            optional<std::map<uint32_t, extended_limit_order>> open_orders;
            optional<std::vector<account_balance_object>> balances;
            optional<std::vector<call_order_object>> call_orders;
            optional<std::vector<force_settlement_object>> settle_orders;
            optional<std::vector<asset_symbol_type>> assets;
            optional<std::vector<std::string>> comments; /// permlinks for this user
            optional<std::vector<std::string>> blog; /// blog posts for this user
            optional<std::vector<std::string>> feed; /// feed posts for this user
            optional<std::vector<std::string>> recent_replies; /// blog posts for this user
            std::map<std::string, std::vector<std::string>> blog_category; /// blog posts for this user
            optional<std::vector<std::string>> recommended; /// posts recommened for this user
        };


        struct candle_stick {
            time_point_sec open_time;
            uint32_t period = 0;
            double high = 0;
            double low = 0;
            double open = 0;
            double close = 0;
            double steem_volume = 0;
            double dollar_volume = 0;
        };

        struct order_history_item {
            time_point_sec time;
            std::string type; // buy or sell
            asset<0, 17, 0> sbd_quantity;
            asset<0, 17, 0> steem_quantity;
            double real_price = 0;
        };

        struct market {
            std::vector<extended_limit_order> bids;
            std::vector<extended_limit_order> asks;
            std::vector<order_history_item> history;
            std::vector<int> available_candlesticks;
            std::vector<int> available_zoom;
            int current_candlestick = 0;
            int current_zoom = 0;
            std::vector<candle_stick> price_history;
        };

        /**
         *  This struct is designed
         */
        struct state {
            std::string current_route;

            dynamic_global_property_object props;

            /**
             *  Tracks the top categories by name, any category in this index
             *  will have its full status stored in the categories map.
             */
            category_index category_idx;

            tag_index tag_idx;

            /**
             * "" is the global discussion index, otherwise the indicies are ranked by category
             */
            std::map<std::string, discussion_index> discussion_idx;

            std::map<std::string, category_api_object> categories;
            std::map<std::string, tag_api_object> tags;

            /**
             *  map from account/slug to full nested discussion
             */
            std::map<std::string, discussion> content;
            std::map<std::string, extended_account> accounts;

            /**
             * The list of miners who are queued to produce work
             */
            std::vector<account_name_type> pow_queue;
            std::map<std::string, witness_api_object> witnesses;
            witness_schedule_object witness_schedule;
            price<0, 17, 0> feed_price;
            std::string error;
            optional<market> market_data;
        };
    }
}

FC_REFLECT_DERIVED((golos::application::extended_account), ((golos::application::account_api_object)),
                   (vesting_balance)(reputation)(transfer_history)(market_history)(post_history)(vote_history)(
                           other_history)(witness_votes)(tags_usage)(guest_bloggers)(open_orders)(comments)(feed)(blog)(
                           recent_replies)(blog_category)(recommended)(balances))


FC_REFLECT((golos::application::vote_state), (voter)(weight)(rshares)(percent)(reputation)(time));
FC_REFLECT((golos::application::account_vote), (authorperm)(weight)(rshares)(percent)(time));

FC_REFLECT((golos::application::discussion_index),
           (category)(trending)(payout)(payout_comments)(trending30)(updated)(created)(responses)(active)(votes)(
                   maturing)(best)(hot)(promoted)(cashout))
FC_REFLECT((golos::application::category_index), (active)(recent)(best))
FC_REFLECT((golos::application::tag_index), (trending))
FC_REFLECT_DERIVED((golos::application::discussion), ((golos::application::comment_api_object)),
                   (url)(root_title)(pending_payout_value)(total_pending_payout_value)(active_votes)(replies)(
                           author_reputation)(promoted)(body_length)(reblogged_by)(first_reblogged_by)(
                           first_reblogged_on))

FC_REFLECT((golos::application::state),
           (current_route)(props)(category_idx)(tag_idx)(categories)(tags)(content)(accounts)(pow_queue)(witnesses)(
                   discussion_idx)(witness_schedule)(feed_price)(error)(market_data))

FC_REFLECT_DERIVED((golos::application::extended_limit_order), ((golos::application::limit_order_object)),
                   (real_price)(rewarded))
FC_REFLECT((golos::application::order_history_item), (time)(type)(sbd_quantity)(steem_quantity)(real_price));
FC_REFLECT((golos::application::market),
           (bids)(asks)(history)(price_history)(available_candlesticks)(available_zoom)(current_candlestick)(
                   current_zoom))
FC_REFLECT((golos::application::candle_stick),
           (open_time)(period)(high)(low)(open)(close)(steem_volume)(dollar_volume));
