#ifndef GOLOS_WORKER_STOEAGE_HPP
#define GOLOS_WORKER_STOEAGE_HPP

#include <memory>
#include "smart_worker.hpp"

namespace steemit {
    namespace chain {
        namespace dynamic_extension {
            class worker_storage final {
            public:
                worker_storage();
                worker_storage(const worker_storage&)= default;
                worker_storage&operator=(const worker_storage&)= default;
                worker_storage(worker_storage&&)= default;
                worker_storage&operator=(worker_storage&&)= default;
                ~worker_storage()= default;

                void add(worker_t *);

                void add(smart_worker);

                view_t get(const std::string &);

            private:
                struct impl;
                std::shared_ptr<impl> pimpl;
            };
        }
    }
}

#endif //GOLOS_SMART_WORKER_HPP
