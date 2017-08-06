#ifndef GOLOS_ASSET_OBJECT_POLICY_HPP
#define GOLOS_ASSET_OBJECT_POLICY_HPP

#include <steemit/chain/database/database_worker.hpp>
#include "steemit/chain/database/generic_policy.hpp"
#include "steemit/chain/dynamic_extension/worker.hpp"
#include <steemit/chain/chain_objects/steem_objects.hpp>

namespace steemit {
namespace chain {
struct asset_policy : public generic_policy {
public:
    asset_policy(const asset_policy &) = default;

    asset_policy &operator=(const asset_policy &) = default;

    asset_policy(asset_policy &&) = default;

    asset_policy &operator=(asset_policy &&) = default;

    virtual ~asset_policy() = default;

    asset_policy(database_basic &ref,int);

    void adjust_supply(const asset &delta, bool adjust_vesting);

    /**
*  Iterates over all conversion requests with a conversion date before
*  the head block time and then converts them to/from steem/sbd at the
*  current median price feed history price times the premium
*/
    void process_conversions();

};



}}
#endif //GOLOS_ASSET_OBJECT_POLICY_HPP
