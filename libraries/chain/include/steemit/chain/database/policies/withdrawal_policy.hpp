#ifndef GOLOS_WITHDRAWAL_POLICY_HPP
#define GOLOS_WITHDRAWAL_POLICY_HPP

#include "steemit/chain/database/generic_policy.hpp"
#include <steemit/chain/chain_objects/steem_object_types.hpp>

namespace steemit {
namespace chain {
struct withdrawal_policy : public generic_policy {

    withdrawal_policy(const withdrawal_policy &) = default;

    withdrawal_policy &operator=(const withdrawal_policy &) = default;

    withdrawal_policy(withdrawal_policy &&) = default;

    withdrawal_policy &operator=(withdrawal_policy &&) = default;

    virtual ~withdrawal_policy() = default;

    withdrawal_policy(database_basic &ref,int);

    void process_vesting_withdrawals();

    const savings_withdraw_object &get_savings_withdraw(const account_name_type &owner, uint32_t request_id) const;

    const savings_withdraw_object *find_savings_withdraw(const account_name_type &owner, uint32_t request_id) const;

};
}}
#endif //GOLOS_WITHDRAWAL_POLICY_HPP
