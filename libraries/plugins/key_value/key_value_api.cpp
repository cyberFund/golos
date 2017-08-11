#include <steemit/key_value/key_value_api.hpp>

namespace steemit {
    namespace key_value {

        namespace detail {

            class key_value_api_impl {
            public:
                key_value_api_impl(steemit::application::application &app)
                        : _app(app) {
                }

                steemit::application::application &_app;
            };

        } // detail

        key_value_api::key_value_api(const steemit::application::api_context &ctx) {
            my = std::make_shared<detail::key_value_api_impl>(ctx.app);
        }

        void key_value_api::on_api_startup() {
        }

    }
} // steemit::key_value