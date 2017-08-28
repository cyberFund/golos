#pragma once

#include <fc/container/flat.hpp>
#include <steemit/protocol/operations/operations.hpp>
#include <steemit/protocol/transaction.hpp>
#include <steemit/chain/steem_object_types.hpp>

#include <fc/string.hpp>

namespace steemit {
    namespace application {

        using namespace fc;

        void operation_get_impacted_accounts(
                const steemit::protocol::operation &op,
                fc::flat_set<protocol::account_name_type> &result);

        void transaction_get_impacted_accounts(
                const steemit::protocol::transaction &tx,
                fc::flat_set<protocol::account_name_type> &result
        );

    }
} // steemit::application
