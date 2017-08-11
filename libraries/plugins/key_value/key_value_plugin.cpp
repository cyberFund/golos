#include <steemit/key_value/key_value_plugin.hpp>
#include <steemit/key_value/key_value_operations.hpp>

#include <steemit/chain/operation_notification.hpp>

#include <steemit/application/plugin.hpp>

namespace steemit {
    namespace key_value {

        namespace detail {

            class key_value_plugin_impl {
            public:
                key_value_plugin_impl(key_value_plugin &_plugin)
                        : self(_plugin) {
                }

                steemit::chain::database &database() {
                    return self.database();
                }

                void pre_operation(const operation_notification &op_obj);

                void post_operation(const operation_notification &op_obj);
            };

            struct pre_operation_visitor {
                key_value_plugin &_plugin;

                pre_operation_visitor(key_value_plugin &plugin)
                        : _plugin(plugin) {
                }

                typedef void result_type;

                template<typename T>
                void operator()(const T &) const {
                }

                void operator()(const create_first_key_value_operation &op) const {

                }

                void operator()(const update_first_key_value_operation &op) const {

                }
            };

            struct post_operation_visitor {
                key_value_plugin &_plugin;

                post_operation_visitor(key_value_plugin &plugin)
                        : _plugin(plugin) {
                }

                typedef void result_type;

                template<typename T>
                void operator()(const T &) const {
                }

                void operator()(const create_first_key_value_operation &op) const {

                }
            };

            void key_value_plugin_impl::pre_operation(const operation_notification &note) {
                note.op.visit(pre_operation_visitor(self));
            }

            void key_value_plugin_impl::post_operation(const operation_notification &note) {
                note.op.visit(post_operation_visitor(self));
            }

        } // detail

        key_value_plugin::key_value_plugin(steemit::application::application *app)
                : plugin(app),
                  my(new detail::key_value_plugin_impl(*this)) {
        }

        void key_value_plugin::plugin_set_program_options(
                boost::program_options::options_description &cli,
                boost::program_options::options_description &cfg
        ) {
        }

        void key_value_plugin::plugin_initialize(const boost::program_options::variables_map &options) {
            try {
                ilog("Initializing key_value plugin");
                chain::database &db = database();

                db.pre_apply_operation.connect([&](const operation_notification &o) { my->pre_operation(o); });
                db.post_apply_operation.connect([&](const operation_notification &o) { my->post_operation(o); });

                db.add_plugin_index<key_lookup_index>();
            }
            FC_CAPTURE_AND_RETHROW()
        }

        void key_value_plugin::plugin_startup() {
            app().register_api_factory<key_value_api>("key_value_api");
        }
    }
} // steemit::key_value

STEEMIT_DEFINE_PLUGIN(key_value, steemit::key_value::key_value_plugin)
