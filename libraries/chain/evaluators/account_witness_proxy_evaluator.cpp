#include <steemit/chain/evaluators/account_witness_proxy_evaluator.hpp>

void steemit::chain::account_witness_proxy_evaluator::do_apply(const protocol::account_witness_proxy_operation &o) {

    const auto &account = this->_db.get_account(o.account);
    FC_ASSERT(account.proxy != o.proxy, "Proxy must change.");

    FC_ASSERT(account.can_vote, "Account has declined the ability to vote and cannot proxy votes.");

    /// remove all current votes
    std::array<share_type, STEEMIT_MAX_PROXY_RECURSION_DEPTH + 1> delta;
    delta[0] = -account.vesting_shares.amount;
    for (int i = 0; i < STEEMIT_MAX_PROXY_RECURSION_DEPTH; ++i) {
        delta[i + 1] = -account.proxied_vsf_votes[i];
    }
    this->_db.adjust_proxied_witness_votes(account, delta);

    if (o.proxy.size()) {
        const auto &new_proxy = this->_db.get_account(o.proxy);
        flat_set<account_id_type> proxy_chain({account.id, new_proxy.id});
        proxy_chain.reserve(STEEMIT_MAX_PROXY_RECURSION_DEPTH + 1);

        /// check for proxy loops and fail to update the proxy if it would create a loop
        auto cprox = &new_proxy;
        while (cprox->proxy.size() != 0) {
            const auto next_proxy = this->_db.get_account(cprox->proxy);
            FC_ASSERT(proxy_chain.insert(next_proxy.id).second, "This proxy would create a proxy loop.");
            cprox = &next_proxy;
            FC_ASSERT(proxy_chain.size() <= STEEMIT_MAX_PROXY_RECURSION_DEPTH, "Proxy chain is too long.");
        }

        /// clear all individual vote records
        this->_db.clear_witness_votes(account);

        this->_db.modify(account, [&](account_object &a) {
            a.proxy = o.proxy;
        });

        /// add all new votes
        for (int i = 0; i <= STEEMIT_MAX_PROXY_RECURSION_DEPTH; ++i) {
            delta[i] = -delta[i];
        }
        this->_db.adjust_proxied_witness_votes(account, delta);
    } else { /// we are clearing the proxy which means we simply update the account
        this->_db.modify(account, [&](account_object &a) {
            a.proxy = o.proxy;
        });
    }
}
