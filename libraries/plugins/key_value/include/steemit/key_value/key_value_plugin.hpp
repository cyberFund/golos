#pragma once

#include <steemit/application/plugin.hpp>
#include <steemit/chain/database.hpp>

#include <steemit/key_value/key_value_api.hpp>

namespace steemit {
    namespace key_value {

#define KEY_VALUE_PLUGIN_NAME "key_value"

        namespace detail { class key_value_plugin_impl; }

        class key_value_plugin : public steemit::application::plugin {
        public:
            key_value_plugin(steemit::application::application *app);

            std::string plugin_name() const override {
                return KEY_VALUE_PLUGIN_NAME;
            }

            virtual void plugin_set_program_options(
                    boost::program_options::options_description &cli,
                    boost::program_options::options_description &cfg) override;

            virtual void plugin_initialize(const boost::program_options::variables_map &options) override;

            virtual void plugin_startup() override;

            friend class detail::key_value_plugin_impl;

            std::unique_ptr<detail::key_value_plugin_impl> my;
        };

    }
} // steemit::key_value
