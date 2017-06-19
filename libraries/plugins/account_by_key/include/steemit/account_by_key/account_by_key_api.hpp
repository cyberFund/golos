#pragma once

#include <steemit/application/application.hpp>

#include <steemit/account_by_key/account_by_key_objects.hpp>

#include <fc/api.hpp>

namespace steemit {
    namespace account_by_key {
        class account_by_key_api: public std::enable_shared_from_this<account_by_key_api> {
        public:
            account_by_key_api(const application::api_context &ctx);

            ~account_by_key_api();

            void on_api_startup();

            vector<vector<account_name_type>> get_key_references(vector<public_key_type> keys) const;

        private:
            struct impl;
            std::unique_ptr<impl> pimpl;
        };

    }
} // steemit::account_by_key

FC_API(steemit::account_by_key::account_by_key_api, (get_key_references))
