#ifndef GOLOS_POW_EVALUATOR_HPP
#define GOLOS_POW_EVALUATOR_HPP

#include "forward.hpp"

namespace steemit {
    namespace chain {

        class pow_evaluator : public evaluator_impl<database_tag, pow_evaluator> {
        public:
            typedef protocol::pow_operation operation_type;

            template<typename Database> pow_evaluator(Database &db) : evaluator_impl<database_tag, pow_evaluator>(db) {
            }

            template<typename Operation> void pow_apply(database_tag &db, Operation o) {
                const auto &dgp = db.get_dynamic_global_properties();

                if (db.is_producing() || db.has_hardfork(STEEMIT_HARDFORK_0_5__59)) {
                    const auto &witness_by_work = db.get_index<witness_index>().indices().get<by_work>();
                    auto work_itr = witness_by_work.find(o.work.work);
                    if (work_itr != witness_by_work.end()) {
                        FC_ASSERT(!"DUPLICATE WORK DISCOVERED", "${w}  ${witness}", ("w", o)("wit", *work_itr));
                    }
                }

                const auto &accounts_by_name = db.get_index<account_index>().indices().get<by_name>();

                auto itr = accounts_by_name.find(o.get_worker_account());
                if (itr == accounts_by_name.end()) {
                    db.create<account_object>([&](account_object &acc) {
                        acc.name = o.get_worker_account();
                        acc.memo_key = o.work.worker;
                        acc.created = dgp.time;
                        acc.last_vote_time = dgp.time;

                        if (!db.has_hardfork(STEEMIT_HARDFORK_0_11__169)) {
                            acc.recovery_account = STEEMIT_INIT_MINER_NAME;
                        } else {
                            acc.recovery_account = "";
                        } /// highest voted witness at time of recovery
                    });

                    db.create<account_authority_object>([&](account_authority_object &auth) {
                        auth.account = o.get_worker_account();
                        auth.owner = authority(1, o.work.worker, 1);
                        auth.active = auth.owner;
                        auth.posting = auth.owner;
                    });
                }

                const auto &worker_account = db.get_account(o.get_worker_account()); // verify it exists
                const auto &worker_auth = db.get<account_authority_object, by_account>(o.get_worker_account());
                FC_ASSERT(worker_auth.active.num_auths() == 1, "Miners can only have one key authority. ${a}",
                          ("a", worker_auth.active));
                FC_ASSERT(worker_auth.active.key_auths.size() == 1, "Miners may only have one key authority.");
                FC_ASSERT(worker_auth.active.key_auths.begin()->first == o.work.worker,
                          "Work must be performed by key that signed the work.");
                FC_ASSERT(o.block_id == db.head_block_id(), "pow not for last block");
                if (db.has_hardfork(STEEMIT_HARDFORK_0_13__256)) {
                    FC_ASSERT(worker_account.last_account_update < db.head_block_time(),
                              "Worker account must not have updated their account this block.");
                }

                fc::sha256 target = db.get_pow_target();

                FC_ASSERT(o.work.work < target, "Work lacks sufficient difficulty.");

                db.modify(dgp, [&](dynamic_global_property_object &p) {
                    p.total_pow++; // make sure this doesn't break anything...
                    p.num_pow_witnesses++;
                });


                const witness_object *cur_witness = db.find_witness(worker_account.name);
                if (cur_witness) {
                    FC_ASSERT(cur_witness->pow_worker == 0,
                              "This account is already scheduled for pow block production.");
                    db.modify(*cur_witness, [&](witness_object &w) {
                        w.props = o.props;
                        w.pow_worker = dgp.total_pow;
                        w.last_work = o.work.work;
                    });
                } else {
                    db.create<witness_object>([&](witness_object &w) {
                        w.owner = o.get_worker_account();
                        w.props = o.props;
                        w.signing_key = o.work.worker;
                        w.pow_worker = dgp.total_pow;
                        w.last_work = o.work.work;
                    });
                }
                /// POW reward depends upon whether we are before or after MINER_VOTING kicks in
                asset pow_reward = db.get_pow_reward();
                if (db.head_block_num() < STEEMIT_START_MINER_VOTING_BLOCK) {
                    pow_reward.amount *= STEEMIT_MAX_WITNESSES;
                }
                db.adjust_supply(pow_reward, true);

                /// pay the witness that includes this POW
                const auto &inc_witness = db.get_account(dgp.current_witness);
                if (db.head_block_num() < STEEMIT_START_MINER_VOTING_BLOCK) {
                    db.adjust_balance(inc_witness, pow_reward);
                } else {
                    db.create_vesting(inc_witness, pow_reward);
                }
            }

            void do_apply(const protocol::pow_operation &o);
        };
    }
}
#endif //GOLOS_POW_EVALUATOR_HPP
