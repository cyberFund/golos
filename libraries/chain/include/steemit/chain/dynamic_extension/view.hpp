#ifndef GOLOS_VIEW_HPP
#define GOLOS_VIEW_HPP

#include "forward.hpp"

namespace steemit {
    namespace chain {
        namespace dynamic_extension {
            class view_t {
            public:
                view_t() = delete;

                view_t(const view_t &) = default;

                view_t &operator=(const view_t &)= default;

                view_t(view_t &&) = default;

                view_t &operator=(view_t &&)= default;

                ~view_t() = default;

                explicit view_t(abstract_worker_t *ptr) : ptr(ptr) {
                }

                abstract_worker_t *operator->() {
                    return ptr;
                }

                abstract_worker_t *operator->() const {
                    return ptr;
                }

            protected:
                abstract_worker_t *ptr;
            };
        }
    }
}
#endif //GOLOS_VIEW_HPP
