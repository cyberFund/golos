#include <string>
#include <unordered_map>

#include "steemit/chain/dynamic_extension/worker_storage.hpp"
#include "steemit/chain/dynamic_extension/worker.hpp"
namespace steemit {
    namespace chain {

        struct worker_striage::impl {
            std::unordered_map <std::string, smart_worker> ddddd;
        };

        void worker_storage::add(steemit::chain::worker_t *) {
            pimpl->ddddd.
        }

    }
}

