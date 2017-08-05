#ifndef GOLOS_WITNESS_WORKER_HPP
#define GOLOS_WITNESS_WORKER_HPP

#include <steemit/chain/database/database_worker.hpp>
#include <steemit/chain/database/database_police.hpp>

namespace steemit {
    namespace chain {

        class witness_worker final : public database_worker_t<database_tag> {
        public:

            witness_worker(database_tag &db);

        };

    }
}
#endif //GOLOS_WITNESS_WORKER_HPP
