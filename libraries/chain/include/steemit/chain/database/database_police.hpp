#pragma once

#include <steemit/chain/database/database_basic.hpp>
#include <steemit/chain/chain_objects/steem_objects.hpp>
#include <steemit/chain/chain_objects/history_object.hpp>
#include <steemit/chain/chain_objects/transaction_object.hpp>
#include <steemit/chain/chain_objects/block_summary_object.hpp>
#include <steemit/chain/operation_notification.hpp>

#include <steemit/chain/database/policies/account_policy.hpp>
#include <steemit/chain/database/policies/asset_policy.hpp>
#include <steemit/chain/database/policies/behaviour_based_policy.hpp>
#include <steemit/chain/database/policies/comment_policy.hpp>
#include <steemit/chain/database/policies/order_policy.hpp>
#include <steemit/chain/database/policies/reward_policy.hpp>
#include <steemit/chain/database/policies/withdrawal_policy.hpp>
#include <steemit/chain/database/policies/witness_policy.hpp>
#include <steemit/chain/database/policies/witness_schedule_policy.hpp>


namespace steemit {
    namespace chain {


        template<typename... Policies>
        class database_police : public database_basic, public Policies ... {
        public:
            database_police(dynamic_extension::worker_storage storage, hard_fork_transformer &&hft)
                    :database_basic(storage,std::move(hft)), Policies(*this)... {


            }

            virtual ~database_police() = default;

        };

        using database_tag = database_police<
                account_policy,
                asset_policy,
                behaviour_based_policy,
                comment_policy,
                order_policy,
                reward_policy,
                withdrawal_policy,
                witness_policy,
                witness_schedule_policy
        >;

    }
}