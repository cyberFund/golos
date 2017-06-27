#pragma once

#include <steemit/application/plugin.hpp>

#include <steemit/chain/database.hpp>
#include <steemit/chain/comment_object.hpp>

#include <boost/multi_index/composite_key.hpp>

#include <fc/thread/future.hpp>
#include <fc/api.hpp>

#include <steemit/comment_metadata/comment_metadata.hpp>

namespace steemit {
    namespace application {
        class discussion_query;

        struct comment_api_obj;
    }

    namespace tags {
        using namespace steemit::chain;
        using namespace boost::multi_index;

        using steemit::application::application;

        using chainbase::object;
        using chainbase::oid;
        using chainbase::allocator;

//
// Plugins should #define their SPACE_ID's so plugins with
// conflicting SPACE_ID assignments can be compiled into the
// same binary (by simply re-assigning some of the conflicting #defined
// SPACE_ID's in a build script).
//
// Assignment of SPACE_ID's cannot be done at run-time because
// various template automagic depends on them being known at compile
// time.
//
#ifndef TAG_SPACE_ID
#define TAG_SPACE_ID 5
#endif

#define TAGS_PLUGIN_NAME "tags"


// Plugins need to define object type IDs such that they do not conflict
// globally. If each plugin uses the upper 8 bits as a space identifier,
// with 0 being for chain, then the lower 8 bits are free for each plugin
// to define as they see fit.
        enum {
            tag_object_type = (TAG_SPACE_ID << 8),
            tag_stats_object_type = (TAG_SPACE_ID << 8) + 1,
            peer_stats_object_type = (TAG_SPACE_ID << 8) + 2,
            author_tag_stats_object_type = (TAG_SPACE_ID << 8) + 3
        };

        namespace detail { class tags_plugin_impl; }

        using tag_object = comment_metadata::comment_meta_data_object<tag_object_type>;

        using tag_index = comment_metadata::comment_meta_data_index<tag_object_type>;

        using tag_stats_object = comment_metadata::comment_metadata_stats_object<tag_stats_object_type>;

        using tag_stats_index = comment_metadata::comment_meta_data_stats_index<tag_stats_object_type>;

        using peer_stats_object = comment_metadata::peer_stats_object<peer_stats_object_type>;

        using peer_stats_index = comment_metadata::peer_stats_index<peer_stats_object_type>;

        using author_tag_stats_object = comment_metadata::author_comment_metadata_stats_object<author_tag_stats_object_type>;

        using author_tag_stats_index = comment_metadata::author_language_stats_index<author_tag_stats_object_type>;

        using comment_metadata::by_tag;
        using comment_metadata::by_author_tag_posts;
        using comment_metadata::by_comment;
        using comment_metadata::by_voter_peer;
        using comment_metadata::by_author_posts_tag;
        using comment_metadata::by_trending;
        using by_author_comment = comment_metadata::by_author_comment<tag_object_type>;
        using by_parent_trending = comment_metadata::by_parent_trending<tag_object_type>;
        using by_parent_promoted = comment_metadata::by_parent_promoted<tag_object_type>;
        using by_reward_fund_net_rshares = comment_metadata::by_reward_fund_net_rshares<tag_object_type>;
        using by_parent_created = comment_metadata::by_parent_created<tag_object_type>;
        using by_parent_active = comment_metadata::by_parent_active<tag_object_type>;
        using by_cashout = comment_metadata::by_cashout<tag_object_type>;
        using by_net_rshares = comment_metadata::by_net_rshares<tag_object_type>;
        using by_parent_net_votes = comment_metadata::by_parent_net_votes<tag_object_type>;
        using by_parent_children = comment_metadata::by_parent_children<tag_object_type>;
        using by_parent_hot = comment_metadata::by_parent_hot<tag_object_type>;


/**
 * Used to parse the metadata from the comment json_meta field.
 */
        struct comment_metadata_t {
            set<string> tags;
        };

/**
 *  This plugin will scan all changes to posts and/or their meta data and
 *
 */
        class tags_plugin : public steemit::application::plugin {
        public:
            tags_plugin(application *app);

            virtual ~tags_plugin();

            std::string plugin_name() const override {
                return TAGS_PLUGIN_NAME;
            }

            virtual void plugin_set_program_options(
                    boost::program_options::options_description &cli,
                    boost::program_options::options_description &cfg) override;

            virtual void plugin_initialize(const boost::program_options::variables_map &options) override;

            virtual void plugin_startup() override;

            static bool filter(const steemit::application::discussion_query &query, const steemit::application::comment_api_obj &c, const std::function<bool(const steemit::application::comment_api_obj &)> &condition);

            friend class detail::tags_plugin_impl;

            std::unique_ptr<detail::tags_plugin_impl> my;
        };

/**
 *  This API is used to query data maintained by the tags_plugin
 */
        class tag_api : public std::enable_shared_from_this<tag_api> {
        public:
            tag_api() {
            };

            tag_api(const steemit::application::api_context &ctx) {
            }//:_app(&ctx.app){}

            void on_api_startup() {
            }

            vector<tag_stats_object> get_tags() const {
                return vector<tag_stats_object>();
            }

        private:
            //application::application* _app = nullptr;
        };
    }
} //steemit::tag

FC_API(steemit::tags::tag_api, (get_tags));

FC_REFLECT(steemit::tags::tag_object,
        (id)(name)(created)(active)(cashout)(net_rshares)(net_votes)(hot)(trending)(promoted_balance)(children)(children_rshares2)(author)(parent)(comment))

CHAINBASE_SET_INDEX_TYPE(steemit::tags::tag_object, steemit::tags::tag_index)

FC_REFLECT(steemit::tags::tag_stats_object, (id)(name)(total_children_rshares2)(total_payout)(net_votes)(top_posts)(comments));
CHAINBASE_SET_INDEX_TYPE(steemit::tags::tag_stats_object, steemit::tags::tag_stats_index)

FC_REFLECT(steemit::tags::peer_stats_object,
        (id)(voter)(peer)(direct_positive_votes)(direct_votes)(indirect_positive_votes)(indirect_votes)(rank));
CHAINBASE_SET_INDEX_TYPE(steemit::tags::peer_stats_object, steemit::tags::peer_stats_index)

FC_REFLECT(steemit::tags::comment_metadata_t, (tags));

FC_REFLECT(steemit::tags::author_tag_stats_object, (id)(author)(name)(total_posts)(total_rewards))
CHAINBASE_SET_INDEX_TYPE(steemit::tags::author_tag_stats_object, steemit::tags::author_tag_stats_index)
