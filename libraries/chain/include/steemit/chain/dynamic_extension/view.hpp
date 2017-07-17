#ifndef GOLOS_VIEW_HPP
#define GOLOS_VIEW_HPP

#include "forward.hpp"

namespace steemit {
    namespace chain {

        class view_t {
        public:
            view_t(worker_t *ptr) : ptr(ptr) {}

        protected:

            worker_t *ptr;
        };
    }
}
#endif //GOLOS_VIEW_HPP
