#ifndef GOLOS_WITNESS_SCHEDULE_POLICY_HPP
#define GOLOS_WITNESS_SCHEDULE_POLICY_HPP

#include <algorithm>

#include "steemit/chain/database/generic_policy.hpp"
#include <steemit/chain/chain_objects/steem_object_types.hpp>

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

    account_name_type get_scheduled_witness(uint32_t slot_num) const;

    const escrow_object &get_escrow(const account_name_type &name, uint32_t escrow_id) const;

    const escrow_object *find_escrow(const account_name_type &name, uint32_t escrow_id) const;

    const witness_schedule_object &get_witness_schedule_object() const;

/**
 *
 *  See @ref witness_object::virtual_last_update
 */
    void update_witness_schedule();

};
}}
#endif //GOLOS_WITNESS_SCHEDULE_POLICY_HPP
