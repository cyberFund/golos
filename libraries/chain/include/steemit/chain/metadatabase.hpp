#pragma once
#include <steemit/chain/evaluator_registry.hpp>
#include <steemit/chain/index.hpp>
#include <steemit/chain/steem_evaluator.hpp>
#include <steemit/chain/database.hpp>
#include <steemit/chain/steem_objects.hpp>
#include <steemit/chain/history_object.hpp>
#include <steemit/chain/transaction_object.hpp>
#include <steemit/chain/block_summary_object.hpp>
#include <steemit/chain/operation_notification.hpp>

#include <steemit/chain/policies/account_policy.hpp>

namespace steemit {
namespace chain {

struct account_write_police {
    explicit account_write_police(database_basic &ref, int f) : references(ref) {
        std::cout << "account_write_police" << std::endl;
    }

    account_write_police() = default;

    account_write_police(const account_write_police &) = default;

    account_write_police &operator=(const account_write_police &) = default;

    account_write_police(account_write_police &&) = default;

    account_write_police &operator=(account_write_police &&) = default;

    virtual ~account_write_police() = default;

    database_basic &references;

};

template<typename... Policies>
class database_police final : public database_basic, public Policies ... {
public:
    database_police(): _evaluator_registry(*this), Policies(*this,_evaluator_registry)...{}

    ~database_police() = default;
    void initialize_indexes(){
        add_core_index<database_basic,dynamic_global_property_index>(*this);
        add_core_index<database_basic,witness_index>(*this);
        add_core_index<database_basic,transaction_index>(*this);
        add_core_index<database_basic,block_summary_index>(*this);
        add_core_index<database_basic,witness_schedule_index>(*this);
        add_core_index<database_basic,comment_index>(*this);
        add_core_index<database_basic,comment_vote_index>(*this);
        add_core_index<database_basic,witness_vote_index>(*this);
        add_core_index<database_basic,limit_order_index>(*this);
        add_core_index<database_basic,feed_history_index>(*this);
        add_core_index<database_basic,convert_request_index>(*this);
        add_core_index<database_basic,liquidity_reward_balance_index>(*this);
        add_core_index<database_basic,operation_index>(*this);
        add_core_index<database_basic,account_history_index>(*this);
        add_core_index<database_basic,hardfork_property_index>(*this);
        add_core_index<database_basic,withdraw_vesting_route_index>(*this);
        add_core_index<database_basic,owner_authority_history_index>(*this);
        add_core_index<database_basic,change_recovery_account_request_index>(*this);
        add_core_index<database_basic,escrow_index>(*this);
        add_core_index<database_basic,savings_withdraw_index>(*this);
        add_core_index<database_basic,decline_voting_rights_request_index>(*this);
        add_core_index<database_basic,vesting_delegation_index>(*this);
        add_core_index<database_basic,vesting_delegation_expiration_index>(*this);
        add_core_index<database_basic,reward_fund_index>(*this);

        _plugin_index_signal();
    }

    void initialize_evaluators(){
        _evaluator_registry.register_evaluator<vote_evaluator>();
        _evaluator_registry.register_evaluator<transfer_evaluator>();
        _evaluator_registry.register_evaluator<transfer_to_vesting_evaluator>();
        _evaluator_registry.register_evaluator<withdraw_vesting_evaluator>();
        _evaluator_registry.register_evaluator<set_withdraw_vesting_route_evaluator>();
        _evaluator_registry.register_evaluator<witness_update_evaluator>();
        _evaluator_registry.register_evaluator<account_witness_vote_evaluator>();
        _evaluator_registry.register_evaluator<account_witness_proxy_evaluator>();
        _evaluator_registry.register_evaluator<custom_evaluator>();
        _evaluator_registry.register_evaluator<custom_binary_evaluator>();
        _evaluator_registry.register_evaluator<custom_json_evaluator>();
        _evaluator_registry.register_evaluator<pow_evaluator>();
        _evaluator_registry.register_evaluator<pow2_evaluator>();
        _evaluator_registry.register_evaluator<report_over_production_evaluator>();
        _evaluator_registry.register_evaluator<feed_publish_evaluator>();
        _evaluator_registry.register_evaluator<convert_evaluator>();
        _evaluator_registry.register_evaluator<challenge_authority_evaluator>();
        _evaluator_registry.register_evaluator<prove_authority_evaluator>();
        _evaluator_registry.register_evaluator<request_account_recovery_evaluator>();
        _evaluator_registry.register_evaluator<recover_account_evaluator>();
        _evaluator_registry.register_evaluator<change_recovery_account_evaluator>();
        _evaluator_registry.register_evaluator<escrow_transfer_evaluator>();
        _evaluator_registry.register_evaluator<escrow_approve_evaluator>();
        _evaluator_registry.register_evaluator<escrow_dispute_evaluator>();
        _evaluator_registry.register_evaluator<escrow_release_evaluator>();
        _evaluator_registry.register_evaluator<transfer_to_savings_evaluator>();
        _evaluator_registry.register_evaluator<transfer_from_savings_evaluator>();
        _evaluator_registry.register_evaluator<cancel_transfer_from_savings_evaluator>();
        _evaluator_registry.register_evaluator<decline_voting_rights_evaluator>();
        _evaluator_registry.register_evaluator<reset_account_evaluator>();
        _evaluator_registry.register_evaluator<set_reset_account_evaluator>();
        _evaluator_registry.register_evaluator<account_create_with_delegation_evaluator>();
        _evaluator_registry.register_evaluator<delegate_vesting_shares_evaluator>();
    }

    void apply_operation(const operation &op){
        operation_notification note(op);
        notify_pre_apply_operation(note);
        _evaluator_registry.get_evaluator(op).apply(op);
        notify_post_apply_operation(note);
    }

protected:
    evaluator_registry<operation> _evaluator_registry;

};

using database = database_police<account_policy, account_write_police>;

}
}