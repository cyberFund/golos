#pragma once

#include <golos/application/application.hpp>

#include <golos/account_by_key/account_by_key_objects.hpp>

#include <fc/api.hpp>

namespace golos {
    namespace account_by_key {

        namespace detail {
            class account_by_key_api_impl;
        }

        /**
         * @brief Used for indexing an retrieving account_object by account_authority_object.
         * @ingroup api
         *
         * This API is intended to retrieve accounts by their keys. This API is
         * read-only; all modifications to the database must be performed via transactions.
         */
        class account_by_key_api {
        public:
            account_by_key_api(const application::api_context &ctx);

            void on_api_startup();

            std::vector<std::vector<account_name_type>> get_key_references(std::vector<public_key_type> keys) const;

        private:
            std::shared_ptr<detail::account_by_key_api_impl> my;
        };

    }
} // golos::account_by_key

FC_API(golos::account_by_key::account_by_key_api, (get_key_references))
