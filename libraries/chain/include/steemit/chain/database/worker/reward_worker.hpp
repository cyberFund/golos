#ifndef GOLOS_REWARD_WORKER_HPP
#define GOLOS_REWARD_WORKER_HPP

#include <steemit/chain/database/database_worker.hpp>
#include <steemit/chain/chain_objects/steem_objects.hpp>
#include <steemit/chain/database/database_policy.hpp>

namespace steemit {
    namespace chain {

        class reward_worker final : public database_worker_t<database_t> {
        public:
            reward_worker(database_t &);

        };
    }
}
#endif //GOLOS_REWARD_WORKER_HPP
