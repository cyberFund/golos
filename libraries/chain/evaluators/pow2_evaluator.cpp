#include <steemit/chain/evaluators/pow2_evaluator.hpp>

void steemit::chain::pow2_evaluator::do_apply(const protocol::pow2_operation &o) {
    auto &db = this->db();
    const auto &dgp = db.get_dynamic_global_properties();
    uint32_t target_pow = db.get_pow_summary_target();
    account_name_type worker_account;

    if (db.has_hardfork(STEEMIT_HARDFORK_0_16__551)) {
        const auto &work = o.work.get<equihash_pow>();
        FC_ASSERT(work.prev_block == db.head_block_id(), "Equihash pow op not for last block");
        auto recent_block_num = protocol::block_header::num_from_id(work.input.prev_block);
        FC_ASSERT(recent_block_num > dgp.last_irreversible_block_num,
                  "Equihash pow done for block older than last irreversible block num");
        FC_ASSERT(work.pow_summary < target_pow, "Insufficient work difficulty. Work: ${w}, Target: ${t}",
                  ("w", work.pow_summary)("t", target_pow));
        worker_account = work.input.worker_account;
    } else {
        const auto &work = o.work.get<pow2>();
        FC_ASSERT(work.input.prev_block == db.head_block_id(), "Work not for last block");
        FC_ASSERT(work.pow_summary < target_pow, "Insufficient work difficulty. Work: ${w}, Target: ${t}",
                  ("w", work.pow_summary)("t", target_pow));
        worker_account = work.input.worker_account;
    }

    FC_ASSERT(o.props.maximum_block_size >= STEEMIT_MIN_BLOCK_SIZE_LIMIT * 2, "Voted maximum block size is too small.");

    db.modify(dgp, [&](dynamic_global_property_object &p) {
        p.total_pow++;
        p.num_pow_witnesses++;
    });

    const auto &accounts_by_name = db.get_index<account_index>().indices().get<by_name>();
    auto itr = accounts_by_name.find(worker_account);
    if (itr == accounts_by_name.end()) {
        FC_ASSERT(o.new_owner_key.valid(), "New owner key is not valid.");
        db.create<account_object>([&](account_object &acc) {
            acc.name = worker_account;
            acc.memo_key = *o.new_owner_key;
            acc.created = dgp.time;
            acc.last_vote_time = dgp.time;
            acc.recovery_account = ""; /// highest voted witness at time of recovery
        });

        db.create<account_authority_object>([&](account_authority_object &auth) {
            auth.account = worker_account;
            auth.owner = authority(1, *o.new_owner_key, 1);
            auth.active = auth.owner;
            auth.posting = auth.owner;
        });

        db.create<witness_object>([&](witness_object &w) {
            w.owner = worker_account;
            w.props = o.props;
            w.signing_key = *o.new_owner_key;
            w.pow_worker = dgp.total_pow;
        });
    } else {
        FC_ASSERT(!o.new_owner_key.valid(), "Cannot specify an owner key unless creating account.");
        const witness_object *cur_witness = db.find_witness(worker_account);
        FC_ASSERT(cur_witness, "Witness must be created for existing account before mining.");
        FC_ASSERT(cur_witness->pow_worker == 0, "This account is already scheduled for pow block production.");
        db.modify(*cur_witness, [&](witness_object &w) {
            w.props = o.props;
            w.pow_worker = dgp.total_pow;
        });
    }

    if (!db.has_hardfork(STEEMIT_HARDFORK_0_16__551)) {
        /// pay the witness that includes this POW
        asset inc_reward = db.get_pow_reward();
        db.adjust_supply(inc_reward, true);

        const auto &inc_witness = db.get_account(dgp.current_witness);
        db.create_vesting(inc_witness, inc_reward);
    }
}
