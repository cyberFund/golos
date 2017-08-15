#pragma once

#include <steemit/chain/database/database_basic.hpp>

namespace steemit {
    namespace chain {

        template<typename Database, typename MultiIndexType> void _add_index_impl(Database &db) {
            db.template add_index<MultiIndexType>();
        }

        template<typename Database, typename MultiIndexType> void add_core_index(Database &db) {
            _add_index_impl<Database, MultiIndexType>(db);
        }

        template<typename Database, typename MultiIndexType> void add_plugin_index(Database &db) {
            db.template _plugin_index_signal.connect([&db]() {
                _add_index_impl<Database, MultiIndexType>(db);
            });
        }

    }
}