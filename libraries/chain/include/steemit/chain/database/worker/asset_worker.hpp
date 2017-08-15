#ifndef GOLOS_ASSET_WORKER_HPP
#define GOLOS_ASSET_WORKER_HPP

#include <steemit/chain/database/database_worker.hpp>
#include <steemit/protocol/config.hpp>
#include <steemit/chain/chain_objects/global_property_object.hpp>
#include <steemit/chain/database/database_policy.hpp>

namespace steemit {
    namespace chain {

        class asset_worker final : public database_worker_t<database_set> {
        public:
            asset_worker(database_set &);
        };
    }
}

#endif //GOLOS_ASSET_WORKER_HPP
