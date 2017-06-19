#pragma once

#include <steemit/account_statistics/account_statistics_plugin.hpp>

#include <fc/api.hpp>

namespace steemit {
    namespace app {
        struct api_context;
    }
}

namespace steemit {
    namespace account_statistics {

        class account_statistics_api:public std::enable_shared_from_this<account_statistics_api> {
        public:
            account_statistics_api(const steemit::application::api_context &ctx);
            ~account_statistics_api();
            void on_api_startup();

        private:
            struct account_statistics_api_impl;
            std::unique_ptr<account_statistics_api_impl> _my;
        };

    }
} // steemit::account_statistics

FC_API(steemit::account_statistics::account_statistics_api,)