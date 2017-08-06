#ifndef GOLOS_ACCOUNT_POLICE_HPP
#define GOLOS_ACCOUNT_POLICE_HPP

#include <steemit/chain/database/generic_policy.hpp>
#include <steemit/chain/chain_objects/steem_objects.hpp>

namespace steemit {
namespace chain {

struct account_policy : public generic_policy {
public:
    account_policy(const account_policy &) = default;

    account_policy &operator=(const account_policy &) = default;

    account_policy(account_policy &&) = default;

    account_policy &operator=(account_policy &&) = default;

    virtual ~account_policy() = default;

    account_policy(database_basic &ref,int);

    asset get_balance(const account_object &a, asset_symbol_type symbol) const;

    asset get_savings_balance(const account_object &a, asset_symbol_type symbol) const;

    void adjust_savings_balance(const account_object &a, const asset &delta);

    void adjust_balance(const account_object &a, const asset &delta);

    void update_owner_authority(const account_object &account, const authority &owner_authority);

    void clear_null_account_balance();

    void pay_fee(const account_object &account, asset fee);

    void old_update_account_bandwidth(const account_object &a, uint32_t trx_size, const bandwidth_type type);

    bool update_account_bandwidth(const account_object &a, uint32_t trx_size, const bandwidth_type type);

    const account_object &get_account(const account_name_type &name) const;

    const account_object *find_account(const account_name_type &name) const;

    void expire_escrow_ratification();
/*
    void adjust_witness_votes(const account_object &a, share_type delta) {
        const auto &vidx = references.get_index<witness_vote_index>().indices().get<by_account_witness>();
        auto itr = vidx.lower_bound(boost::make_tuple(a.id, witness_id_type()));
        while (itr != vidx.end() && itr->account == a.id) {
            storage.get("witness")->invoke<void>("adjust_witness_vote",references.get(itr->witness), delta);
            ++itr;
        }
    }


    /// this updates the votes for all witnesses as a result of account VESTS changing
    void adjust_proxied_witness_votes(const account_object &a, share_type delta, int depth = 0) {
        if (a.proxy != STEEMIT_PROXY_TO_SELF_ACCOUNT) {
            /// nested proxies are not supported, vote will not propagate
            if (depth >= STEEMIT_MAX_PROXY_RECURSION_DEPTH) {
                return;
            }

            const auto &proxy = references.get_account(a.proxy);

            references.modify(proxy, [&](account_object &a) {
                a.proxied_vsf_votes[depth] += delta;
            });

            adjust_proxied_witness_votes(proxy, delta, depth + 1);
        } else {
            adjust_witness_votes(a, delta);
        }
    }

    /// this updates the votes for witnesses as a result of account voting proxy changing
    void adjust_proxied_witness_votes(
            const account_object &a,
            const std::array<share_type,STEEMIT_MAX_PROXY_RECURSION_DEPTH + 1> &delta,
            int depth = 0) {
        if (a.proxy != STEEMIT_PROXY_TO_SELF_ACCOUNT) {
            /// nested proxies are not supported, vote will not propagate
            if (depth >= STEEMIT_MAX_PROXY_RECURSION_DEPTH) {
                return;
            }

            const auto &proxy = references.get_account(a.proxy);

            references.modify(proxy, [&](account_object &a) {
                for (int i = STEEMIT_MAX_PROXY_RECURSION_DEPTH - depth - 1;
                     i >= 0; --i) {
                    a.proxied_vsf_votes[i + depth] += delta[i];
                }
            });

            adjust_proxied_witness_votes(proxy, delta, depth + 1);
        } else {
            share_type total_delta = 0;
            for (int i = STEEMIT_MAX_PROXY_RECURSION_DEPTH - depth;
                 i >= 0; --i) {
                total_delta += delta[i];
            }
            adjust_witness_votes(a, total_delta);
        }
    }
*/
    asset get_balance(const string &aname, asset_symbol_type symbol) const;


    /**
 *  Overall the network has an inflation rate of 102% of virtual steem per year
 *  90% of inflation is directed to vesting shares
 *  10% of inflation is directed to subjective proof of work voting
 *  1% of inflation is directed to liquidity providers
 *  1% of inflation is directed to block producers
 *
 *  This method pays out vesting and reward shares every block, and liquidity shares once per day.
 *  This method does not pay out witnesses.
 */
    void process_funds();

    void process_savings_withdraws();


    asset get_producer_reward();

    void account_recovery_processing();

    void clear_expired_delegations();
};



}}
#endif
