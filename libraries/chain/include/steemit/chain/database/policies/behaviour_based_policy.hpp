#ifndef GOLOS_BEHAVIOUR_BASED_POLICY_HPP
#define GOLOS_BEHAVIOUR_BASED_POLICY_HPP

#include "steemit/chain/database/generic_policy.hpp"
#include <steemit/chain/chain_objects/steem_objects.hpp>

namespace steemit {
    namespace chain {

        struct behaviour_based_policy : public generic_policy {
        public:
            behaviour_based_policy(const behaviour_based_policy &) = default;

            behaviour_based_policy &operator=(const behaviour_based_policy &) = default;

            behaviour_based_policy(behaviour_based_policy &&) = default;

            behaviour_based_policy &operator=(behaviour_based_policy &&) = default;

            virtual ~behaviour_based_policy() = default;

            behaviour_based_policy(database_basic &ref, int);

            /*
                share_type pay_reward_funds(share_type reward){

                }
            */
            /**
         *  Converts STEEM into sbd and adds it to to_account while reducing the STEEM supply
         *  by STEEM and increasing the sbd supply by the specified amount.
         */
            /*
                std::pair<asset, asset> create_sbd(const account_object &to_account, asset steem) {
                    std::pair<asset, asset> assets(asset(0, SBD_SYMBOL), asset(0, STEEM_SYMBOL));

                    try {
                        if (steem.amount == 0) {
                            return assets;
                        }

                        const auto &median_price = references.get_feed_history().current_median_history;
                        const auto &gpo = references.get_dynamic_global_properties();

                        if (!median_price.is_null()) {
                            auto to_sbd = (gpo.sbd_print_rate * steem.amount) /
                                          STEEMIT_100_PERCENT;
                            auto to_steem = steem.amount - to_sbd;

                            auto sbd = asset(to_sbd, STEEM_SYMBOL) * median_price;

                            storage.get("account")->invoke<void>("adjust_balance",to_account, sbd);
                            storage.get("account")->invoke<void>("adjust_balance",to_account, asset(to_steem, STEEM_SYMBOL));
                            storage.get("asset")->invoke<void>("adjust_supply",asset(-to_sbd, STEEM_SYMBOL));
                            storage.get("asset")->invoke<void>("adjust_supply",sbd);
                            assets.first = sbd;
                            assets.second = to_steem;
                        } else {
                            storage.get("account")->invoke<void>("adjust_balance",to_account, steem);
                            assets.second = steem;
                        }
                    }
                    FC_CAPTURE_LOG_AND_RETHROW((to_account.name)(steem))

                    return assets;
                }
            */
            asset get_payout_extension_cost(const comment_object &input_comment,
                                            const fc::time_point_sec &input_time) const;

            fc::sha256 get_pow_target() const;

            void update_median_feed();

            uint32_t get_pow_summary_target() const;
        };


    }
}
#endif
