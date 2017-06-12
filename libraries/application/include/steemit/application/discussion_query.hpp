#ifndef GOLOS_DISCUSSION_QUERY_H
#define GOLOS_DISCUSSION_QUERY_H


#include <fc/api.hpp>
#include <fc/optional.hpp>
#include <fc/variant_object.hpp>
#include <fc/network/ip.hpp>


#include <map>
#include <set>
#include <memory>
#include <vector>
#include <fc/exception/exception.hpp>

namespace steemit {
    namespace application {
/**
 * @class discussion_query
 * @brief The discussion_query structure implements the RPC API param set.
 *  Defines the arguments to a query as a struct so it can be easily extended
 */

        class discussion_query {
        public:
            void validate() const {
                FC_ASSERT(limit <= 100);

                for (const std::set<std::string>::value_type &iterator : filter_tags) {
                    FC_ASSERT(select_tags.find(iterator) == select_tags.end());
                }

                for (const auto &iterator : filter_language) {
                    FC_ASSERT(select_language.find(iterator) == select_language.end());
                }
            }

            uint32_t limit = 0; ///< the discussions return amount top limit
            std::set<std::string> select_authors; ///< list of authors to select
            std::set<std::string> select_tags; ///< list of tags to include, posts without these tags are filtered
            std::set<std::string> filter_tags; ///< list of tags to exclude, posts with these tags are filtered;
            uint32_t truncate_body = 0; ///< the amount of bytes of the post body to return, 0 for all
            optional<std::string> start_author; ///< the author of discussion to start searching from
            optional<std::string> start_permlink; ///< the permlink of discussion to start searching from
            optional<std::string> parent_author; ///< the author of parent discussion
            optional<std::string> parent_permlink; ///< the permlink of parent discussion
            std::set<std::string> select_language; ///< list of language to select
            std::set<std::string> filter_language; ///< list of language to filter
        };
}}
#endif //GOLOS_DISCUSSION_QUERY_H
