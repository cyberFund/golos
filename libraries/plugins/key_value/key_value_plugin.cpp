#include <steemit/key_value/key_value_plugin.hpp>
#include <steemit/key_value/key_value_operations.hpp>

#include <steemit/chain/operation_notification.hpp>
#include <steemit/chain/generic_custom_operation_interpreter.hpp>

#include <steemit/application/plugin.hpp>

namespace steemit {
    namespace key_value {

        namespace detail {

            class key_value_plugin_impl {
            public:
                key_value_plugin_impl(key_value_plugin &_plugin) : self(_plugin) {
                }

                void plugin_initialize();

                steemit::chain::database &database() {
                    return self.database();
                }

                void pre_operation(const operation_notification &op_obj);

                void post_operation(const operation_notification &op_obj);

                key_value_plugin &self;
                std::shared_ptr<generic_custom_operation_interpreter<steemit::key_value::key_value_plugin_operation>> _custom_operation_interpreter;
            };

            struct pre_operation_visitor {
                key_value_plugin &plugin;

                pre_operation_visitor(key_value_plugin &input_plugin) : plugin(input_plugin) {
                }

                typedef void result_type;

                template<typename T> void operator()(const T &) const {
                }

                void operator()(const create_first_key_value_operation &op) const {

                }

                void operator()(const update_first_key_value_operation &op) const {

                }
            };

            struct post_operation_visitor {
                key_value_plugin &plugin;

                post_operation_visitor(key_value_plugin &input_plugin) : plugin(input_plugin) {
                }

                typedef void result_type;

                template<typename T> void operator()(const T &) const {
                }

                void operator()(const create_first_key_value_operation &op) const {

                }

                void operator()(const custom_json_operation &op) const {
                    try {
                        if (op.id == KEY_VALUE_PLUGIN_NAME) {
                            custom_json_operation new_cop;

                            new_cop.required_auths = op.required_auths;
                            new_cop.required_posting_auths = op.required_posting_auths;
                            new_cop.id = _plugin.plugin_name();
                            first_key_value_operation fop;

                            try {
                                fop = fc::json::from_string(op.json).as<first_key_value_operation>();
                            }
                            catch (const fc::exception &) {
                                return;
                            }

                            auto new_fop = key_value_plugin_operation(fop);
                            new_cop.json = fc::json::to_string(new_fop);
                            std::shared_ptr<custom_operation_interpreter> eval = _plugin.database().get_custom_json_evaluator(op.id);
                            eval->apply(new_cop);
                        }
                    }
                    FC_CAPTURE_AND_RETHROW()
                }
            };

            void key_value_plugin_impl::plugin_initialize() {
                _custom_operation_interpreter = std::make_shared<generic_custom_operation_interpreter<steemit::key_value::key_value_plugin_operation>>(database());

                // Add each operation evaluator to the registry
                _custom_operation_interpreter->register_evaluator<create_first_key_value_evaluator>(&self);
                _custom_operation_interpreter->register_evaluator<update_first_key_value_evaluator>(&self);
                _custom_operation_interpreter->register_evaluator<delete_first_key_value_evaluator>(&self);

                // Add the registry to the database so the database can delegate custom ops to the plugin]
                database().set_custom_operation_interpreter(self.plugin_name(), _custom_operation_interpreter);
            }

            void key_value_plugin_impl::pre_operation(const operation_notification &note) {
                try {
                    note.op.visit(pre_operation_visitor(self));
                }
                catch (const fc::assert_exception &) {
                    if (database().is_producing()) {
                        throw;
                    }
                }
            }

            void key_value_plugin_impl::post_operation(const operation_notification &note) {
                try {
                    note.op.visit(post_operation_visitor(self));
                }
                catch (fc::assert_exception) {
                    if (database().is_producing()) {
                        throw;
                    }
                }
            }
        } // detail

        key_value_plugin::key_value_plugin(steemit::application::application *app) : plugin(app), my(new detail::key_value_plugin_impl(*this)) {
        }

        void key_value_plugin::plugin_set_program_options(boost::program_options::options_description &cli,
                                                          boost::program_options::options_description &cfg) {
        }

        void key_value_plugin::plugin_initialize(const boost::program_options::variables_map &options) {
            try {
                ilog("Initializing key_value plugin");
                chain::database &db = database();
                my->plugin_initialize();

                db.pre_apply_operation.connect([&](const operation_notification &o) {
                    my->pre_operation(o);
                });
                db.post_apply_operation.connect([&](const operation_notification &o) {
                    my->post_operation(o);
                });

                db.add_plugin_index<key_value_first_index>();
                db.add_plugin_index<key_value_second_index>();
            } FC_CAPTURE_AND_RETHROW()
        }

        void key_value_plugin::plugin_startup() {
            app().register_api_factory<key_value_api>("key_value_api");
        }
    }
} // steemit::key_value

STEEMIT_DEFINE_PLUGIN(key_value, steemit::key_value::key_value_plugin)
