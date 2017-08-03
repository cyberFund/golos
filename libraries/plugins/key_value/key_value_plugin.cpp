#include <steemit/key_value/key_value_plugin.hpp>
#include <steemit/key_value/key_value_operations.hpp>

#include <steemit/chain/operation_notification.hpp>

#include <steemit/application/plugin.hpp>

namespace steemit {
    namespace key_value_store {

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

        void key_value_plugin::update_key_lookup(const account_authority_object &a) {
            auto &db = database();
            flat_set<public_key_type> new_keys;

            // Construct the set of keys in the account's authority
            for (const auto &item : a.owner.key_auths) {
                new_keys.insert(item.first);
            }
            for (const auto &item : a.active.key_auths) {
                new_keys.insert(item.first);
            }
            for (const auto &item : a.posting.key_auths) {
                new_keys.insert(item.first);
            }

            // For each key that needs a lookup
            for (const auto &key : new_keys) {
                // If the key was not in the authority, add it to the lookup
                if (my->cached_keys.find(key) == my->cached_keys.end()) {
                    auto lookup_itr = db.find<key_lookup_object, by_key>(std::make_tuple(key, a.account));

                    if (lookup_itr == nullptr) {
                        db.create<key_lookup_object>([&](key_lookup_object &o) {
                            o.key = key;
                            o.account = a.account;
                        });
                    }
                } else {
                    // If the key was already in the auths, remove it from the set so we don't delete it
                    my->cached_keys.erase(key);
                }
            }

            // Loop over the keys that were in authority but are no longer and remove them from the lookup
            for (const auto &key : my->cached_keys) {
                auto lookup_itr = db.find<key_lookup_object, by_key>(std::make_tuple(key, a.account));

                if (lookup_itr != nullptr) {
                    db.remove(*lookup_itr);
                }
            }

            my->cached_keys.clear();
        }
    }
} // steemit::key_value

STEEMIT_DEFINE_PLUGIN(key_value_store, steemit::key_value_store::key_value_plugin)
