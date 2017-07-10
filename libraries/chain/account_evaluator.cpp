#include <steemit/chain/account_evaluator.hpp>

#include <steemit/chain/steem_evaluator.hpp>
#include <steemit/chain/database.hpp>
#include <steemit/chain/custom_operation_interpreter.hpp>
#include <steemit/chain/steem_objects.hpp>
#include <steemit/chain/block_summary_object.hpp>
#include <steemit/chain/utilities/reward.hpp>

namespace steemit {
    namespace chain {

        void account_create_evaluator::do_apply(const account_create_operation &o) {
            const auto &creator = db.get_account(o.creator);

            const auto &props = db.get_dynamic_global_properties();

            FC_ASSERT(db.get_balance(creator.name, STEEM_SYMBOL) >=
                      o.fee, "Insufficient balance to create account.", ("creator.balance", db.get_balance(creator.name, STEEM_SYMBOL))("required", o.fee));

            if (db.has_hardfork(STEEMIT_HARDFORK_0_17__101)) {
                const witness_schedule_object &wso = db.get_witness_schedule_object();
                FC_ASSERT(o.fee >=
                          asset(wso.median_props.account_creation_fee.amount *
                                STEEMIT_CREATE_ACCOUNT_WITH_STEEM_MODIFIER, STEEM_SYMBOL), "Insufficient Fee: ${f} required, ${p} provided.",
                        ("f", wso.median_props.account_creation_fee *
                              asset(STEEMIT_CREATE_ACCOUNT_WITH_STEEM_MODIFIER, STEEM_SYMBOL))
                                ("p", o.fee));
            } else if (db.has_hardfork(STEEMIT_HARDFORK_0_1)) {
                const witness_schedule_object &wso = db.get_witness_schedule_object();
                FC_ASSERT(o.fee >=
                          wso.median_props.account_creation_fee, "Insufficient Fee: ${f} required, ${p} provided.",
                        ("f", wso.median_props.account_creation_fee)
                                ("p", o.fee));
            }

            if (db.is_producing() ||
                db.has_hardfork(STEEMIT_HARDFORK_0_15__465)) {
                for (auto &a : o.owner.account_auths) {
                    db.get_account(a.first);
                }

                for (auto &a : o.active.account_auths) {
                    db.get_account(a.first);
                }

                for (auto &a : o.posting.account_auths) {
                    db.get_account(a.first);
                }
            }

            db.adjust_balance(creator, -o.fee);

            const auto &new_account = db.create<account_object>([&](account_object &acc) {
                acc.name = o.new_account_name;
                acc.memo_key = o.memo_key;
                acc.created = props.time;
                acc.last_vote_time = props.time;
                acc.mined = false;

                if (!db.has_hardfork(STEEMIT_HARDFORK_0_11__169)) {
                    acc.recovery_account = STEEMIT_INIT_MINER_NAME;
                } else {
                    acc.recovery_account = o.creator;
                }


#ifndef IS_LOW_MEM
                from_string(acc.json_metadata, o.json_metadata);
#endif
            });

            db.create<account_authority_object>([&](account_authority_object &auth) {
                auth.account = o.new_account_name;
                auth.owner = o.owner;
                auth.active = o.active;
                auth.posting = o.posting;
                auth.last_owner_update = fc::time_point_sec::min();
            });

            if (o.fee.amount > 0) {
                db.create_vesting(new_account, o.fee);
            }

            db.create<account_statistics_object>([&](account_statistics_object &s) {
                s.owner = new_account.name;
            });
        }

        void account_create_with_delegation_evaluator::do_apply(const account_create_with_delegation_operation &o) {

            FC_ASSERT(db.has_hardfork(STEEMIT_HARDFORK_0_17__101), "Account creation with delegation is not enabled until hardfork 17");

            const auto &creator = db.get_account(o.creator);
            const auto &props = db.get_dynamic_global_properties();
            const witness_schedule_object &wso = db.get_witness_schedule_object();

            FC_ASSERT(creator.balance >=
                      o.fee, "Insufficient balance to create account.",
                    ("creator.balance", creator.balance)
                            ("required", o.fee));

            FC_ASSERT(
                    creator.vesting_shares - creator.delegated_vesting_shares -
                    asset(creator.to_withdraw -
                          creator.withdrawn, VESTS_SYMBOL) >=
                    o.delegation, "Insufficient vesting shares to delegate to new account.",
                    ("creator.vesting_shares", creator.vesting_shares)
                            ("creator.delegated_vesting_shares", creator.delegated_vesting_shares)("required", o.delegation));

            auto target_delegation =
                    asset(wso.median_props.account_creation_fee.amount *
                          STEEMIT_CREATE_ACCOUNT_WITH_STEEM_MODIFIER *
                          STEEMIT_CREATE_ACCOUNT_DELEGATION_RATIO, STEEM_SYMBOL) *
                    props.get_vesting_share_price();

            auto current_delegation = asset(o.fee.amount *
                                            STEEMIT_CREATE_ACCOUNT_DELEGATION_RATIO, STEEM_SYMBOL) *
                                      props.get_vesting_share_price() +
                                      o.delegation;

            FC_ASSERT(current_delegation >=
                      target_delegation, "Inssufficient Delegation ${f} required, ${p} provided.",
                    ("f", target_delegation)
                            ("p", current_delegation)
                            ("account_creation_fee", wso.median_props.account_creation_fee)
                            ("o.fee", o.fee)
                            ("o.delegation", o.delegation));

            FC_ASSERT(o.fee >=
                      wso.median_props.account_creation_fee, "Insufficient Fee: ${f} required, ${p} provided.",
                    ("f", wso.median_props.account_creation_fee)
                            ("p", o.fee));

            for (auto &a : o.owner.account_auths) {
                db.get_account(a.first);
            }

            for (auto &a : o.active.account_auths) {
                db.get_account(a.first);
            }

            for (auto &a : o.posting.account_auths) {
                db.get_account(a.first);
            }

            db.adjust_balance(creator, -o.fee);

            db.modify(creator, [&](account_object &c) {
                c.delegated_vesting_shares += o.delegation;
            });

            const auto &new_account = db.create<account_object>([&](account_object &acc) {
                acc.name = o.new_account_name;
                acc.memo_key = o.memo_key;
                acc.created = props.time;
                acc.last_vote_time = props.time;
                acc.mined = false;

                acc.recovery_account = o.creator;

                acc.received_vesting_shares = o.delegation;

#ifndef IS_LOW_MEM
                from_string(acc.json_metadata, o.json_metadata);
#endif
            });

            db.create<account_authority_object>([&](account_authority_object &auth) {
                auth.account = o.new_account_name;
                auth.owner = o.owner;
                auth.active = o.active;
                auth.posting = o.posting;
                auth.last_owner_update = fc::time_point_sec::min();
            });

            if (o.delegation.amount > 0) {
                db.create<vesting_delegation_object>([&](vesting_delegation_object &vdo) {
                    vdo.delegator = o.creator;
                    vdo.delegatee = o.new_account_name;
                    vdo.vesting_shares = o.delegation;
                    vdo.min_delegation_time = db.head_block_time() +
                                              STEEMIT_CREATE_ACCOUNT_DELEGATION_TIME;
                });
            }

            if (o.fee.amount > 0) {
                db.create_vesting(new_account, o.fee);
            }
        }

        void account_update_evaluator::do_apply(const account_update_operation &o) {

            if (db.has_hardfork(STEEMIT_HARDFORK_0_1)) {
                FC_ASSERT(o.account !=
                          STEEMIT_TEMP_ACCOUNT, "Cannot update temp account.");
            }

            if ((db.has_hardfork(STEEMIT_HARDFORK_0_15__465) ||
                 db.is_producing()) && o.posting) { // TODO: Add HF 15
                o.posting->validate();
            }

            const auto &account = db.get_account(o.account);
            const auto &account_auth = db.get<account_authority_object, by_account>(o.account);

            if (o.owner) {
#ifndef STEEMIT_BUILD_TESTNET
                if (db.has_hardfork(STEEMIT_HARDFORK_0_11)) {
                    FC_ASSERT(db.head_block_time() -
                              account_auth.last_owner_update >
                              STEEMIT_OWNER_UPDATE_LIMIT, "Owner authority can only be updated once an hour.");
                }

#endif

                if ((db.has_hardfork(STEEMIT_HARDFORK_0_15__465) ||
                     db.is_producing())) // TODO: Add HF 15
                {
                    for (auto a: o.owner->account_auths) {
                        db.get_account(a.first);
                    }
                }


                db.update_owner_authority(account, *o.owner);
            }

            if (o.active &&
                (db.has_hardfork(STEEMIT_HARDFORK_0_15__465) ||
                 db.is_producing())) // TODO: Add HF 15
            {
                for (auto a: o.active->account_auths) {
                    db.get_account(a.first);
                }
            }

            if (o.posting &&
                (db.has_hardfork(STEEMIT_HARDFORK_0_15__465) ||
                 db.is_producing())) // TODO: Add HF 15
            {
                for (auto a: o.posting->account_auths) {
                    db.get_account(a.first);
                }
            }

            db.modify(account, [&](account_object &acc) {
                if (o.memo_key != public_key_type()) {
                    acc.memo_key = o.memo_key;
                }

                if ((o.active || o.owner) && acc.active_challenged) {
                    acc.active_challenged = false;
                    acc.last_active_proved = db.head_block_time();
                }

                acc.last_account_update = db.head_block_time();

#ifndef IS_LOW_MEM
                if (o.json_metadata.size() > 0) {
                    from_string(acc.json_metadata, o.json_metadata);
                }
#endif
            });

            if (o.active || o.posting) {
                db.modify(account_auth, [&](account_authority_object &auth) {
                    if (o.active) {
                        auth.active = *o.active;
                    }
                    if (o.posting) {
                        auth.posting = *o.posting;
                    }
                });
            }
        }

        void account_whitelist_evaluator::do_apply(const protocol::account_whitelist_operation &o) {
            try {
                listed_account = db.get_account(o.account_to_list);
            } FC_CAPTURE_AND_RETHROW((o))

            try {

                db.modify(listed_account, [&o](account_object &a) {
                    if (o.new_listing & o.white_listed) {
                        a.whitelisting_accounts.insert(o.authorizing_account);
                    } else {
                        a.whitelisting_accounts.erase(o.authorizing_account);
                    }

                    if (o.new_listing & o.black_listed) {
                        a.blacklisting_accounts.insert(o.authorizing_account);
                    } else {
                        a.blacklisting_accounts.erase(o.authorizing_account);
                    }
                });

                /** for tracking purposes only, this state is not needed to evaluate */
                db.modify(db.get_account(o.authorizing_account), [&](account_object &a) {
                    if (o.new_listing & o.white_listed) {
                        a.whitelisted_accounts.insert(o.account_to_list);
                    } else {
                        a.whitelisted_accounts.erase(o.account_to_list);
                    }

                    if (o.new_listing & o.black_listed) {
                        a.blacklisted_accounts.insert(o.account_to_list);
                    } else {
                        a.blacklisted_accounts.erase(o.account_to_list);
                    }
                });
            } FC_CAPTURE_AND_RETHROW((o))
        }
    }
}