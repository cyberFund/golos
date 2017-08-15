#ifndef GOLOS_WITNESS_WORKER_HPP
#define GOLOS_WITNESS_WORKER_HPP

#include <steemit/chain/database/database_worker.hpp>
#include <steemit/chain/database/database_policy.hpp>

namespace steemit {
    namespace chain {

        class witness_worker final : public database_worker_t<database_set> {
        public:

            witness_worker(database_set &db);

        };

    }
}
#endif //GOLOS_WITNESS_WORKER_HPP
