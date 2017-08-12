#pragma once

#include <steemit/application/application.hpp>

#include <steemit/key_value/key_value_objects.hpp>

#include <fc/api.hpp>

namespace steemit {
    namespace key_value {

        namespace detail {
            class key_value_api_impl;
        }

        class key_value_api {
        public:
            key_value_api(const application::api_context &ctx);

            void on_api_startup();

            vector <first_key_value_object> get_first_object_by_system(const std::string &system);

            vector <first_key_value_object> get_first_object_by_block_number(fc::uint128_t number);

            vector <first_key_value_object> get_first_object_by_hash(const std::string &hash);

            vector <first_key_value_object> get_first_object_by_ipfs_hash(const std::string &ipfs_hash);

            vector <first_key_value_object> get_first_object_by_account(const std::string &account);

            vector <first_key_value_object> get_first_object_by_time_range(const fc::time_point_sec &start,
                                                                           const fc::time_point_sec &end);
        private:
            std::shared_ptr<detail::key_value_api_impl> my;
        };

    }
} // steemit::key_value

FC_API(steemit::key_value::key_value_api, (get_first_object_by_system)(get_first_object_by_block_number)(get_first_object_by_hash)(get_first_object_by_ipfs_hash)(get_first_object_by_account)(get_first_object_by_time_range))
