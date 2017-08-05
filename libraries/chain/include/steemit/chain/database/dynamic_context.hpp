#ifndef GOLOS_DYNAMIC_CONTEXT_HPP
#define GOLOS_DYNAMIC_CONTEXT_HPP

#include "hard_fork_transformer.hpp"
#include "../dynamic_extension/worker_storage.hpp"

namespace steemit {
    namespace chain {

        class dynamic_context {

        private:
            struct impl;
            std::shared_ptr<impl>pimpl;
        };
    }
}
#endif //GOLOS_DYNAMIC_CONTEXT_HPP
