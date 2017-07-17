#ifndef GOLOS_SMART_WORKER_HPP
#define GOLOS_SMART_WORKER_HPP

#include <memory>

#include "forward.hpp"

#include "smart_worker.hpp"

namespace steemit {
    namespace chain {


        class worker_storage {
        public:

            void add(worker_t *);

            void add(smart_worker *);

        private:
            struct impl;
            std::shared_ptr<impl>pimpl;
        };
    }
}
#endif //GOLOS_SMART_WORKER_HPP
