#pragma once

#include <steemit/chain/database.hpp>

namespace steemit {
    namespace chain {

        template<typename DataBase, typename MultiIndexType>
        void _add_index_impl(DataBase &db) {
            db.template add_index<MultiIndexType>();
        }

        template<typename DataBase,typename MultiIndexType>
        void add_core_index(DataBase  &db) {
            _add_index_impl<DataBase,MultiIndexType>(db);
        }

        template<typename DataBase,typename MultiIndexType>
        void add_plugin_index(DataBase  &db) {
            db.template _plugin_index_signal.connect([&db]() { _add_index_impl<DataBase,MultiIndexType>(db); });
        }

    }
}
