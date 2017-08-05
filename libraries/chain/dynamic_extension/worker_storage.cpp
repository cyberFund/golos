#include <string>
#include <unordered_map>

#include "steemit/chain/dynamic_extension/worker_storage.hpp"

namespace steemit {
    namespace chain {
        namespace dynamic_extension {
            struct worker_storage::impl final {
                std::unordered_map<std::string, smart_worker> storage;
            };

            void worker_storage::add(worker_t *w) {
                pimpl->storage.emplace(w->name(), w);
            }

            void worker_storage::add(smart_worker w) {
                pimpl->storage.emplace(w.name(), w);
            }

            view_t worker_storage::get(const std::string &key) {
                return pimpl->storage.at(key).view();
            }

        }
    }
}

