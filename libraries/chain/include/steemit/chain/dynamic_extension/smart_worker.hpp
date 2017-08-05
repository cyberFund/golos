#ifndef GOLOS_SMART_WORKER_HPP
#define GOLOS_SMART_WORKER_HPP

#include "worker.hpp"
#include "view.hpp"

namespace steemit {
    namespace chain {
        namespace dynamic_extension {
            class smart_worker {
            public:
                smart_worker()= delete;
                smart_worker(const smart_worker&)= default;
                smart_worker&operator=(const smart_worker&)= default;
                smart_worker(smart_worker&&)= default;
                smart_worker&operator=(smart_worker&&)= default;
                ~smart_worker()= default;

                smart_worker(abstract_worker_t *);

                view_t view() const;

                const std::string name() const;

                abstract_worker_t *operator->();

                abstract_worker_t *operator->() const;

            private:
                abstract_worker_t *ptr;
            };

        }
    }
}

#endif //GOLOS_SMART_WORKER_HPP
