#ifndef GOLOS_REWARD_WORKER_HPP
#define GOLOS_REWARD_WORKER_HPP

#include <steemit/chain/database/database_worker.hpp>
#include <steemit/chain/steem_objects.hpp>
#include <steemit/chain/database/database_police.hpp>

namespace steemit {
    namespace chain {

        class reward_worker final : public database_worker_t<database_tag> {
        public:
            reward_worker(database_tag&);

        };
    }
}
#endif //GOLOS_REWARD_WORKER_HPP
