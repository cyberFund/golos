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

            vector<vector<account_name_type>> get_key_references(vector<public_key_type> keys) const;

        private:
            std::shared_ptr<detail::key_value_api_impl> my;
        };

    }
} // steemit::key_value

FC_API(steemit::key_value::key_value_api, (get_key_references))
