#ifndef GOLOS_GENERIC_POLICY_HPP
#define GOLOS_GENERIC_POLICY_HPP

#include <steemit/chain/dynamic_extension/worker_storage.hpp>

namespace steemit {
    namespace chain {

        class database_basic;

        class generic_policy {
        public:
            generic_policy(database_basic &references);

            virtual ~generic_policy() = default;

        protected:
            database_basic &references;

        };
    }
}

#endif //GOLOS_GENERIC_POLICY_HPP
