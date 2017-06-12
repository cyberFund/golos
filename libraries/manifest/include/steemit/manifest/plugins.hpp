
#pragma once

#include <memory>
#include <string>
#include <vector>

namespace steemit {
    namespace application {

        class abstract_plugin;

        class application;

    }
}

namespace steemit {
    namespace plugin {

        void initialize_plugin_factories();

        std::shared_ptr<steemit::application::abstract_plugin> create_plugin(const std::string &name, steemit::application::application *app);

        std::vector<std::string> get_available_plugins();

    }
}
