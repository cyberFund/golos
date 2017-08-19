#ifndef GOLOS_BIG_HELPER_HPP
#define GOLOS_BIG_HELPER_HPP

#include <steemit/chain/chain_objects/steem_objects.hpp>
#include <steemit/chain/chain_objects/steem_object_types.hpp>


namespace steemit {
    namespace chain {
        class database_basic;
        namespace database_helper {
            namespace big_helper {

                const account_object &get_account(database_basic &db, const account_name_type &name);


                const witness_schedule_object &get_witness_schedule_object(database_basic &db);


                account_name_type get_scheduled_witness(database_basic &db, uint32_t slot_num);


                const witness_object &get_witness(database_basic &db, const account_name_type &name);

                void adjust_balance(database_basic&database,const account_object &a, const asset &delta);


                bool update_account_bandwidth(database_basic&database,const account_object &a, uint32_t trx_size, const bandwidth_type type);

                void old_update_account_bandwidth(database_basic&database,const account_object &a, uint32_t trx_size, const bandwidth_type type);

                void adjust_supply(database_basic&database,const asset &delta, bool adjust_vesting = false);


                share_type pay_reward_funds(database_basic&database,share_type reward);


///  Converts STEEM into sbd and adds it to to_account while reducing the STEEM supply
///  by STEEM and increasing the sbd supply by the specified amount.

                std::pair<asset, asset> create_sbd(database_basic&database,const account_object &to_account, asset steem);

                asset to_sbd(database_basic&database,const asset &steem);

                asset to_steem(database_basic&database,const asset &sbd);

                void adjust_liquidity_reward(database_basic&database,const account_object &owner, const asset &volume, bool is_sdb);


                void adjust_witness_vote(
                        database_basic&database,
                        const witness_object &witness,
                        share_type delta
                );


                void adjust_witness_votes(database_basic&database, const account_object &a, share_type delta);


                /** this updates the votes for all witnesses as a result of account VESTS changing */
                void
                adjust_proxied_witness_votes(database_basic&database, const account_object &a, share_type delta, int depth = 0);

                /** this updates the votes for witnesses as a result of account voting proxy changing */
                void adjust_proxied_witness_votes(
                        database_basic&database,
                        const account_object &a,
                        const std::array<share_type, STEEMIT_MAX_PROXY_RECURSION_DEPTH + 1> &delta,
                        int depth = 0);

                asset create_vesting(database_basic&database,const account_object &to_account, asset steem);


            }
        }
    }
}
#endif //GOLOS_BIG_HELPER_HPP
