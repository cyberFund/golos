#include <steemit/account_statistics/account_statistics_api.hpp>

namespace steemit {
    namespace account_statistics {

            struct account_statistics_api:: account_statistics_api_impl {
            public:
                account_statistics_api_impl(steemit::application::application &app)
                        : _app(app) {
                }

                steemit::application::application &_app;
            };


        account_statistics_api::account_statistics_api(const steemit::application::api_context &ctx):_my(new account_statistics_api_impl(ctx.app)  ) {
        }

        void account_statistics_api::on_api_startup() {
        }

    account_statistics_api::~account_statistics_api()=default;

    }
} // steemit::account_statistics