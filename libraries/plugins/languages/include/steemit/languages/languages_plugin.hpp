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

    namespace languages {
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
#ifndef LANGUAGES_SPACE_ID
#define LANGUAGES_SPACE_ID 12
#endif

#define LANGUAGES_PLUGIN_NAME "languages"

// Plugins need to define object type IDs such that they do not conflict
// globally. If each plugin uses the upper 8 bits as a space identifier,
// with 0 being for chain, then the lower 8 bits are free for each plugin
// to define as they see fit.
        enum {
            language_object_type = (LANGUAGES_SPACE_ID << 8),
            language_stats_object_type = (LANGUAGES_SPACE_ID << 8) + 1,
            peer_stats_object_type = (LANGUAGES_SPACE_ID << 8) + 2,
            author_language_stats_object_type = (LANGUAGES_SPACE_ID << 8) + 3
        };

        using language_object = comment_metadata::comment_meta_data_object<language_object_type>;

        using language_index = comment_metadata::comment_meta_data_index<language_object_type>;

        using language_stats_object = comment_metadata::comment_metadata_stats_object<language_stats_object_type>;

        using language_stats_index = comment_metadata::comment_meta_data_stats_index<language_stats_object_type>;

        using peer_stats_object = comment_metadata::peer_stats_object<peer_stats_object_type>;

        using peer_stats_index = comment_metadata::peer_stats_index<peer_stats_object_type>;

        using author_language_stats_object = comment_metadata::author_comment_metadata_stats_object<author_language_stats_object_type>;

        using author_language_stats_index = comment_metadata::author_language_stats_index<author_language_stats_object_type>;

        using comment_metadata::by_tag;
        using comment_metadata::by_author_tag_posts;
        using comment_metadata::by_comment;
        using comment_metadata::by_voter_peer;
        using by_author_comment = comment_metadata::by_author_comment<language_object_type>;
        using by_parent_trending = comment_metadata::by_parent_trending<language_object_type>;
        using by_parent_promoted = comment_metadata::by_parent_promoted<language_object_type>;
        using by_reward_fund_net_rshares = comment_metadata::by_reward_fund_net_rshares<language_object_type>;
        using by_parent_created = comment_metadata::by_parent_created<language_object_type>;
        using by_parent_active = comment_metadata::by_parent_active<language_object_type>;
        using by_cashout = comment_metadata::by_cashout<language_object_type>;
        using by_net_rshares = comment_metadata::by_net_rshares<language_object_type>;
        using by_parent_net_votes = comment_metadata::by_parent_net_votes<language_object_type>;
        using by_parent_children = comment_metadata::by_parent_children<language_object_type>;
        using by_parent_hot = comment_metadata::by_parent_hot<language_object_type>;
/**
 * Used to parse the metadata from the comment json_meta field.
 */
        struct comment_metadata_t {
            string language;
        };

/**
 *  This plugin will scan all changes to posts and/or their meta data and
 *
 */
        class languages_plugin : public steemit::application::plugin {
        public:
            languages_plugin(application *app);

            virtual ~languages_plugin();

            std::string plugin_name() const override {
                return LANGUAGES_PLUGIN_NAME;
            }

            virtual void plugin_set_program_options(
                    boost::program_options::options_description &cli,
                    boost::program_options::options_description &cfg) override;

            virtual void plugin_initialize(const boost::program_options::variables_map &options) override;

            virtual void plugin_startup() override;

            static bool filter(const steemit::application::discussion_query &query, const steemit::application::comment_api_obj &c, const std::function<bool(const steemit::application::comment_api_obj &)> &confition);

            const std::set<std::string> get_languages() const;

            struct languages_plugin_impl;

            std::unique_ptr<languages_plugin_impl> my;
        };

/**
 *  This API is used to query data maintained by the languages_plugin
 */
        class language_api : public std::enable_shared_from_this<language_api> {
        public:

            language_api();

            language_api(const steemit::application::api_context &ctx);

            void on_api_startup();

            std::vector<std::string> get_languages() const;

        private:
            struct impl;
            std::unique_ptr<impl> pimpl;
        };
    }
} //steemit::language

FC_REFLECT(steemit::languages::comment_metadata_t, (language));

FC_API(steemit::languages::language_api, (get_languages));

FC_REFLECT(steemit::languages::language_object, (id)(name)(created)(active)(cashout)(net_rshares)(net_votes)(hot)(trending)(promoted_balance)(children)(children_rshares2)(author)(parent)(comment))
CHAINBASE_SET_INDEX_TYPE(steemit::languages::language_object, steemit::languages::language_index)

FC_REFLECT(steemit::languages::language_stats_object,
        (id)(name)(total_children_rshares2)(total_payout)(net_votes)(top_posts)(comments));
CHAINBASE_SET_INDEX_TYPE(steemit::languages::language_stats_object, steemit::languages::language_stats_index)

FC_REFLECT(steemit::languages::peer_stats_object,
        (id)(voter)(peer)(direct_positive_votes)(direct_votes)(indirect_positive_votes)(indirect_votes)(rank));
CHAINBASE_SET_INDEX_TYPE(steemit::languages::peer_stats_object, steemit::languages::peer_stats_index)

FC_REFLECT(steemit::languages::author_language_stats_object, (id)(author)(name)(total_posts)(total_rewards))
CHAINBASE_SET_INDEX_TYPE(steemit::languages::author_language_stats_object, steemit::languages::author_language_stats_index)
