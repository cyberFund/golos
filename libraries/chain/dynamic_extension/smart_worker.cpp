#include <steemit/chain/dynamic_extension/smart_worker.hpp>
#include <cassert>

namespace steemit {
    namespace chain {
        namespace dynamic_extension {

            smart_worker::smart_worker(abstract_worker_t *ptr) : ptr(ptr) {
                assert(ptr != nullptr);
            }

            view_t smart_worker::view() const {
                return view_t(ptr);
            }

            const std::string smart_worker::name() const {
                return ptr->name();
            }

            abstract_worker_t *smart_worker::operator->() {
                return ptr;
            }

            abstract_worker_t *smart_worker::operator->() const {
                return ptr;
            }

        }
    }
}