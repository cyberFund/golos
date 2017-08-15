#ifndef GOLOS_ACCOUNT_WORKER_HPP
#define GOLOS_ACCOUNT_WORKER_HPP

#include <steemit/chain/database/database_worker.hpp>
#include <steemit/chain/database/database_policy.hpp>
#include <steemit/chain/chain_objects/steem_objects.hpp>

namespace steemit {
    namespace chain {

        class account_worker final : public database_worker_t<database_tag> {
        public:
            account_worker(database_tag &db);
        };
    }
}
#endif //GOLOS_ACCOUNT_WORKER_HPP
