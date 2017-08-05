#ifndef GOLOS_WITNESS_SCHEDULE_POLICY_HPP
#define GOLOS_WITNESS_SCHEDULE_POLICY_HPP

#include <algorithm>

#include "steemit/chain/database/generic_policy.hpp"

namespace steemit {
namespace chain {

struct witness_schedule_policy : public generic_policy {
    witness_schedule_policy(const witness_schedule_policy &) = default;

    witness_schedule_policy &operator=(const witness_schedule_policy &) = default;

    witness_schedule_policy(witness_schedule_policy &&) = default;

    witness_schedule_policy &operator=(witness_schedule_policy &&) = default;

    virtual ~witness_schedule_policy() = default;

    witness_schedule_policy(database_basic &ref,int);

    void reset_virtual_schedule_time();

    void update_median_witness_props();

    void update_witness_schedule4();

/**
 *
 *  See @ref witness_object::virtual_last_update
 */
    void update_witness_schedule();

};
}}
#endif //GOLOS_WITNESS_SCHEDULE_POLICY_HPP
