#ifndef GOLOS_BEHAVIOUR_BASED_WORKER_HPP
#define GOLOS_BEHAVIOUR_BASED_WORKER_HPP

#include <steemit/chain/database/database_worker.hpp>
#include <steemit/chain/chain_objects/steem_objects.hpp>
#include <steemit/chain/utilities/asset.hpp>
#include <steemit/chain/database/database_policy.hpp>


namespace steemit {
    namespace chain {
        class behaviour_based_worker : public database_worker_t<database_set> {
        public:
            behaviour_based_worker(database_set &);
        };
    }
}


/**
 *
 * asset to_sbd(const asset &steem) const {
        return utilities::to_sbd(references.get_feed_history().current_median_history, steem);
    }

    asset to_steem(const asset &sbd) const {
        return utilities::to_steem(references.get_feed_history().current_median_history, sbd);
    }*/
#endif //GOLOS_BEHAVIOUR_BASED_WORKER_HPP
