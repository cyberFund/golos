#ifndef GOLOS_TRANSFORMER_HPP
#define GOLOS_TRANSFORMER_HPP

#include <steemit/chain/steem_objects.hpp>
#include <steemit/chain/compound.hpp>

namespace steemit {
    namespace chain {

        template<typename DataBase>
        class transformer : public DataBase {
        public:

            const dynamic_global_property_object &get_dynamic_global_properties() const {
                try {
                    return this->template get<dynamic_global_property_object>();
                }
                FC_CAPTURE_AND_RETHROW()

            }

            account_name_type get_scheduled_witness(uint32_t slot_num) const {
                const dynamic_global_property_object &dpo = get_dynamic_global_properties();
                const witness_schedule_object &wso = get_witness_schedule_object();
                uint64_t current_aslot = dpo.current_aslot + slot_num;
                return wso.current_shuffled_witnesses[current_aslot % wso.num_scheduled_witnesses];
            }

            const feed_history_object &get_feed_history() const {
                try {
                    return this->template get<feed_history_object>();
                } FC_CAPTURE_AND_RETHROW()
            }

            const account_object &get_account(const account_name_type &name) const {
                try {
                    return this->template get<account_object, by_name>(name);
                } FC_CAPTURE_AND_RETHROW((name))
            }

            const account_object *find_account(const account_name_type &name) const {
                return this->template find<account_object, by_name>(name);
            }

            const comment_object &get_comment(const account_name_type &author, const shared_string &permlink) const {
                try {
                     return this->template get<comment_object, by_permlink>(boost::make_tuple(author, permlink));
                } FC_CAPTURE_AND_RETHROW((author)(permlink))
            }

            const comment_object *find_comment(const account_name_type &author, const shared_string &permlink) const {
                return this->template find<comment_object, by_permlink>(boost::make_tuple(author, permlink));
            }

            const comment_object &get_comment(const account_name_type &author, const string &permlink) const {
                try {
                    return this->template get<comment_object, by_permlink>(boost::make_tuple(author, permlink));
                } FC_CAPTURE_AND_RETHROW((author)(permlink))
            }

            const comment_object *find_comment(const account_name_type &author, const string &permlink) const {
                return this->template find<comment_object, by_permlink>(boost::make_tuple(author, permlink));
            }

            const escrow_object &get_escrow(const account_name_type &name, uint32_t escrow_id) const {
                try {
                    return this->template get<escrow_object, by_from_id>(boost::make_tuple(name, escrow_id));
                } FC_CAPTURE_AND_RETHROW((name)(escrow_id))
            }

            const escrow_object *find_escrow(const account_name_type &name, uint32_t escrow_id) const {
                return this->template find<escrow_object, by_from_id>(boost::make_tuple(name, escrow_id));
            }


            const limit_order_object &get_limit_order(const account_name_type &name, uint32_t orderid) const {
                try {
                    ///TODO BIG PRPBLEM
                   // if (!has_hardfork(STEEMIT_HARDFORK_0_6__127)) {
                   //     orderid = orderid & 0x0000FFFF;
                   // }

                    return this->template get<limit_order_object, by_account>(boost::make_tuple(name, orderid));
                } FC_CAPTURE_AND_RETHROW((name)(orderid))
            }

            const limit_order_object *find_limit_order(const account_name_type &name, uint32_t orderid) const {
                ///TODO BIG PRPBLEM
                //if (!has_hardfork(STEEMIT_HARDFORK_0_6__127)) {
                //    orderid = orderid & 0x0000FFFF;
                //}

                return this-> template find<limit_order_object, by_account>(boost::make_tuple(name, orderid));
            }


            const savings_withdraw_object &
            get_savings_withdraw(const account_name_type &owner, uint32_t request_id) const {
                try {
                    return this->template get<savings_withdraw_object, by_from_rid>(boost::make_tuple(owner, request_id));
                } FC_CAPTURE_AND_RETHROW((owner)(request_id))
            }

            const savings_withdraw_object *
            find_savings_withdraw(const account_name_type &owner, uint32_t request_id) const {
                return this->template find<savings_withdraw_object, by_from_rid>(boost::make_tuple(owner, request_id));
            }

            const witness_schedule_object &get_witness_schedule_object() const {
                try {
                    return this->template get<witness_schedule_object>();
                } FC_CAPTURE_AND_RETHROW()
            }

            const witness_object &get_witness(const account_name_type &name) const {
                try {
                    return this->template get<witness_object, by_name>(name);
                } FC_CAPTURE_AND_RETHROW((name))
            }

            const witness_object *find_witness(const account_name_type &name) const {
                return this->template find<witness_object, by_name>(name);
            }

            const reward_fund_object &get_reward_fund(const comment_object &c) const {
                return this-> template get<reward_fund_object, by_name>(
                        c.parent_author == STEEMIT_ROOT_POST_PARENT
                        ? STEEMIT_POST_REWARD_FUND_NAME
                        : STEEMIT_COMMENT_REWARD_FUND_NAME);
            }


        asset get_liquidity_reward() const {
            /** BIG PROBLEM
            if(has_hardfork(STEEMIT_HARDFORK_0_12__178)) {
                return asset(0, STEEM_SYMBOL);
            }
            */

            const auto &props = get_dynamic_global_properties();
            static_assert(STEEMIT_LIQUIDITY_REWARD_PERIOD_SEC == 60 * 60, "this code assumes a 1 hour time interval");
            asset percent(protocol::calc_percent_reward_per_hour<STEEMIT_LIQUIDITY_APR_PERCENT>(props.virtual_supply.amount), STEEM_SYMBOL);
            return std::max(percent, STEEMIT_MIN_LIQUIDITY_REWARD);
        }

            asset get_content_reward() const {
                const auto &props = get_dynamic_global_properties();
                auto reward = asset(255, STEEM_SYMBOL);
                static_assert(STEEMIT_BLOCK_INTERVAL == 3, "this code assumes a 3-second time interval");
                if (props.head_block_number > STEEMIT_START_VESTING_BLOCK) {
                    asset percent(protocol::calc_percent_reward_per_block<STEEMIT_CONTENT_APR_PERCENT>(props.virtual_supply.amount), STEEM_SYMBOL);
                    reward = std::max(percent, STEEMIT_MIN_CONTENT_REWARD);
                }

                return reward;
            }

            asset get_curation_reward() const {
                const auto &props = get_dynamic_global_properties();
                auto reward = asset(85, STEEM_SYMBOL);
                static_assert(STEEMIT_BLOCK_INTERVAL == 3, "this code assumes a 3-second time interval");
                if (props.head_block_number > STEEMIT_START_VESTING_BLOCK) {
                    asset percent(protocol::calc_percent_reward_per_block<STEEMIT_CURATE_APR_PERCENT>(props.virtual_supply.amount), STEEM_SYMBOL);
                    reward = std::max(percent, STEEMIT_MIN_CURATE_REWARD);
                }

                return reward;
            }



            virtual ~transformer() = default;
        };

    }
}
#endif //GOLOS_TRANSFORMER_HPP
