#include <steemit/protocol/steem_operations.hpp>

#include <steemit/chain/chain_objects/block_summary_object.hpp>
#include <steemit/chain/compound.hpp>
#include <steemit/chain/custom_operation_interpreter.hpp>
#include <steemit/chain/database/database_basic.hpp>
#include <steemit/chain/database/database_exceptions.hpp>
#include <steemit/chain/db_with.hpp>
#include <steemit/chain/chain_objects/history_object.hpp>
#include <steemit/chain/chain_objects/steem_objects.hpp>
#include <steemit/chain/chain_objects/transaction_object.hpp>
#include <steemit/chain/shared_db_merkle.hpp>
#include <steemit/chain/operation_notification.hpp>

#include <steemit/chain/utilities/asset.hpp>
#include <steemit/chain/utilities/reward.hpp>
#include <steemit/chain/utilities/uint256.hpp>

#include <fc/smart_ref_impl.hpp>

#include <fc/container/deque.hpp>

#include <fc/io/fstream.hpp>
#include <fc/io/json.hpp>

namespace steemit {
    namespace chain {

        struct object_schema_repr {
            std::pair<uint16_t, uint16_t> space_type;
            std::string type;
        };

        struct operation_schema_repr {
            std::string id;
            std::string type;
        };

        struct db_schema {
            std::map<std::string, std::string> types;
            std::vector<object_schema_repr> object_types;
            std::string operation_type;
            std::vector<operation_schema_repr> custom_operation_types;
        };

    }
}

FC_REFLECT(steemit::chain::object_schema_repr, (space_type)(type))
FC_REFLECT(steemit::chain::operation_schema_repr, (id)(type))
FC_REFLECT(steemit::chain::db_schema, (types)(object_types)(operation_type)(custom_operation_types))

namespace steemit {
    namespace chain {
namespace {
    const witness_schedule_object &get_witness_schedule_object(database_basic &db) {
        try {
            return db.get<witness_schedule_object>();
        } FC_CAPTURE_AND_RETHROW()
    }


    account_name_type get_scheduled_witness(database_basic &db, uint32_t slot_num) {
        const dynamic_global_property_object &dpo = db.get_dynamic_global_properties();
        const witness_schedule_object &wso = get_witness_schedule_object(db);
        uint64_t current_aslot = dpo.current_aslot + slot_num;
        return wso.current_shuffled_witnesses[current_aslot % wso.num_scheduled_witnesses];
    }


    const witness_object &get_witness(database_basic &db, const account_name_type &name) {
        try {
            return db.get<witness_object, by_name>(name);
        } FC_CAPTURE_AND_RETHROW((name))
    }

    const account_object &get_account(database_basic &db, const account_name_type &name) {
        try {
            return db.get<account_object, by_name>(name);
        } FC_CAPTURE_AND_RETHROW((name))
    }


}
        using boost::container::flat_set;

        database_basic::database_basic(){

        }

        database_basic::~database_basic() {
            clear_pending();
        }

        void database_basic::open(const fc::path &data_dir, const fc::path &shared_mem_dir, uint64_t initial_supply, uint64_t shared_file_size, uint32_t chainbase_flags) {
            try {
                init_schema();
                chainbase::database::open(shared_mem_dir, chainbase_flags, shared_file_size);

                initialize_indexes();
                initialize_workers();
                initialize_evaluators();

                if (chainbase_flags & chainbase::database::read_write) {
                    if (!find<dynamic_global_property_object>()) {
                        with_write_lock([&]() {
                            init_genesis(initial_supply);
                        });
                    }

                    _block_log.open(data_dir / "block_log");

                    auto log_head = _block_log.head();

                    // Rewind all undo state. This should return us to the state at the last irreversible block.
                    with_write_lock([&]() {
                        undo_all();
                        FC_ASSERT(revision() == head_block_num(), "Chainbase revision does not match head block num",
                                ("rev", revision())("head_block", head_block_num()));
                    });

                    if (head_block_num()) {
                        auto head_block = _block_log.read_block_by_num(head_block_num());
                        // This assertion should be caught and a reindex should occur
                        FC_ASSERT(head_block.valid() && head_block->id() ==
                                                        head_block_id(), "Chain state does not match block log. Please reindex blockchain.");

                        _fork_db.start_block(*head_block);
                    }
                }

                with_read_lock([&]() {
                    init_hardforks(); // Writes to local state, but reads from db
                });

            }
            FC_CAPTURE_LOG_AND_RETHROW((data_dir)(shared_mem_dir)(shared_file_size))
        }

        void database_basic::reindex(const fc::path &data_dir, const fc::path &shared_mem_dir, uint64_t shared_file_size) {
            try {
                ilog("Reindexing Blockchain");
                wipe(data_dir, shared_mem_dir, false);
                open(data_dir, shared_mem_dir, STEEMIT_INIT_SUPPLY, shared_file_size, chainbase::database::read_write);
                _fork_db.reset();    // override effect of _fork_db.start_block() call in open()

                auto start = fc::time_point::now();
                STEEMIT_ASSERT(_block_log.head(), block_log_exception, "No blocks in block log. Cannot reindex an empty chain.");

                ilog("Replaying blocks...");


                uint64_t skip_flags =
                        static_cast<uint64_t >(validation_steps::skip_witness_signature) |
                        static_cast<uint64_t >(validation_steps::skip_transaction_signatures) |
                        static_cast<uint64_t >(validation_steps::skip_transaction_dupe_check) |
                        static_cast<uint64_t >(validation_steps::skip_tapos_check) |
                        static_cast<uint64_t >(validation_steps::skip_merkle_check) |
                        static_cast<uint64_t >(validation_steps::skip_witness_schedule_check) |
                        static_cast<uint64_t >(validation_steps::skip_authority_check) |
                        static_cast<uint64_t >(validation_steps::skip_validate) | /// no need to validate operations
                        static_cast<uint64_t >(validation_steps::skip_validate_invariants) |
                        static_cast<uint64_t >(validation_steps::skip_block_log);

                with_write_lock([&]() {
                    auto itr = _block_log.read_block(0);
                    auto last_block_num = _block_log.head()->block_num();

                    while (itr.first.block_num() != last_block_num) {
                        auto cur_block_num = itr.first.block_num();
                        if (cur_block_num % 100000 == 0) {
                            std::cerr << "   " << double(cur_block_num * 100) /
                                                  last_block_num << "%   "
                                      << cur_block_num << " of "
                                      << last_block_num <<
                                      "   ("
                                      << (get_free_memory() / (1024 * 1024))
                                      << "M free)\n";
                        }
                        apply_block(itr.first, skip_flags);
                        itr = _block_log.read_block(itr.second);
                    }

                    apply_block(itr.first, skip_flags);
                    set_revision(head_block_num());
                });

                if (_block_log.head()->block_num()) {
                    _fork_db.start_block(*_block_log.head());
                }

                auto end = fc::time_point::now();
                ilog("Done reindexing, elapsed time: ${t} sec", ("t",
                        double((end - start).count()) / 1000000.0));
            }
            FC_CAPTURE_AND_RETHROW((data_dir)(shared_mem_dir))

        }

        void database_basic::wipe(const fc::path &data_dir, const fc::path &shared_mem_dir, bool include_blocks) {
            close();
            chainbase::database::wipe(shared_mem_dir);
            if (include_blocks) {
                fc::remove_all(data_dir / "block_log");
                fc::remove_all(data_dir / "block_log.index");
            }
        }

        void database_basic::close(bool rewind) {
            try {
                // Since pop_block() will move tx's in the popped blocks into pending,
                // we have to clear_pending() after we're done popping to get a clean
                // DB state (issue #336).
                clear_pending();

                chainbase::database::flush();
                chainbase::database::close();

                _block_log.close();

                _fork_db.reset();
            }
            FC_CAPTURE_AND_RETHROW()
        }

        bool database_basic::is_known_block(const block_id_type &id) const {
            try {
                return fetch_block_by_id(id).valid();
            } FC_CAPTURE_AND_RETHROW()
        }

/**
 * Only return true *if* the transaction has not expired or been invalidated. If this
 * method is called with a VERY old transaction we will return false, they should
 * query things by blocks if they are that old.
 */
        bool database_basic::is_known_transaction(const transaction_id_type &id) const {
            try {
                const auto &trx_idx = get_index<transaction_index>().indices().get<by_trx_id>();
                return trx_idx.find(id) != trx_idx.end();
            } FC_CAPTURE_AND_RETHROW()
        }

        block_id_type database_basic::find_block_id_for_num(uint32_t block_num) const {
            try {
                if (block_num == 0) {
                    return block_id_type();
                }

                // Reversible blocks are *usually* in the TAPOS buffer.  Since this
                // is the fastest check, we do it first.
                block_summary_id_type bsid = block_num & 0xFFFF;
                const block_summary_object *bs = find<block_summary_object, by_id>(bsid);
                if (bs != nullptr) {
                    if (protocol::block_header::num_from_id(bs->block_id) ==
                        block_num) {
                        return bs->block_id;
                    }
                }

                // Next we query the block log.   Irreversible blocks are here.

                auto b = _block_log.read_block_by_num(block_num);
                if (b.valid()) {
                    return b->id();
                }

                // Finally we query the fork DB.
                shared_ptr<fork_item> fitem = _fork_db.fetch_block_on_main_branch_by_number(block_num);
                if (fitem) {
                    return fitem->id;
                }

                return block_id_type();
            }
            FC_CAPTURE_AND_RETHROW((block_num))
        }

        block_id_type database_basic::get_block_id_for_num(uint32_t block_num) const {
            block_id_type bid = find_block_id_for_num(block_num);
            FC_ASSERT(bid != block_id_type());
            return bid;
        }

        optional<signed_block> database_basic::fetch_block_by_id(const block_id_type &id) const {
            try {
                auto b = _fork_db.fetch_block(id);
                if (!b) {
                    auto tmp = _block_log.read_block_by_num(protocol::block_header::num_from_id(id));

                    if (tmp && tmp->id() == id) {
                        return tmp;
                    }

                    tmp.reset();
                    return tmp;
                }

                return b->data;
            } FC_CAPTURE_AND_RETHROW()
        }

        optional<signed_block> database_basic::fetch_block_by_number(uint32_t block_num) const {
            try {
                optional<signed_block> b;

                auto results = _fork_db.fetch_block_by_number(block_num);
                if (results.size() == 1) {
                    b = results[0]->data;
                } else {
                    b = _block_log.read_block_by_num(block_num);
                }

                return b;
            } FC_LOG_AND_RETHROW()
        }

        const signed_transaction database_basic::get_recent_transaction(const transaction_id_type &trx_id) const {
            try {
                auto &index = get_index<transaction_index>().indices().get<by_trx_id>();
                auto itr = index.find(trx_id);
                FC_ASSERT(itr != index.end());
                signed_transaction trx;
                fc::raw::unpack(itr->packed_trx, trx);
                return trx;;
            } FC_CAPTURE_AND_RETHROW()
        }

        std::vector<block_id_type> database_basic::get_block_ids_on_fork(block_id_type head_of_fork) const {
            try {
                pair<fork_database::branch_type, fork_database::branch_type> branches = _fork_db.fetch_branch_from(head_block_id(), head_of_fork);
                if (!((branches.first.back()->previous_id() ==
                       branches.second.back()->previous_id()))) {
                    edump((head_of_fork)
                            (head_block_id())
                            (branches.first.size())
                            (branches.second.size()));
                    assert(branches.first.back()->previous_id() ==
                           branches.second.back()->previous_id());
                }
                std::vector<block_id_type> result;
                for (const item_ptr &fork_block : branches.second) {
                    result.emplace_back(fork_block->id);
                }
                result.emplace_back(branches.first.back()->previous_id());
                return result;
            } FC_CAPTURE_AND_RETHROW()
        }

        chain_id_type database_basic::get_chain_id() const {
            return STEEMIT_CHAIN_ID;
        }


        const node_property_object &database_basic::get_node_properties() const {
            return _node_property_object;
        }

        void database_basic::add_checkpoints(const flat_map<uint32_t, block_id_type> &checkpts) {
            for (const auto &i : checkpts) {
                _checkpoints[i.first] = i.second;
            }
        }

        bool database_basic::before_last_checkpoint() const {
            return (_checkpoints.size() > 0) &&
                   (_checkpoints.rbegin()->first >= head_block_num());
        }

/**
 * Push block "may fail" in which case every partial change is unwound.  After
 * push block is successful the block is appended to the chain database_basic on disk.
 *
 * @return true if we switched forks as a result of this push.
 */
        bool database_basic::push_block(const signed_block &new_block, uint32_t skip) {
            //fc::time_point begin_time = fc::time_point::now();

            bool result;
            detail::with_skip_flags(*this, skip, [&]() {
                with_write_lock([&]() {
                    detail::without_pending_transactions(*this, std::move(_pending_tx), [&]() {
                        try {
                            result = _push_block(new_block);
                        }
                        FC_CAPTURE_AND_RETHROW((new_block))
                    });
                });
            });

            //fc::time_point end_time = fc::time_point::now();
            //fc::microseconds dt = end_time - begin_time;
            //if( ( new_block.block_num() % 10000 ) == 0 )
            //   ilog( "push_block ${b} took ${t} microseconds", ("b", new_block.block_num())("t", dt.count()) );
            return result;
        }

        void database_basic::_maybe_warn_multiple_production(uint32_t height) const {
            auto blocks = _fork_db.fetch_block_by_number(height);
            if (blocks.size() > 1) {
                vector<std::pair<account_name_type, fc::time_point_sec>> witness_time_pairs;
                for (const auto &b : blocks) {
                    witness_time_pairs.push_back(std::make_pair(b->data.witness, b->data.timestamp));
                }

                ilog("Encountered block num collision at block ${n} due to a fork, witnesses are:", ("n", height)("w", witness_time_pairs));
            }
            return;
        }

        bool database_basic::_push_block(const signed_block &new_block) {
            try {
                uint32_t skip = get_node_properties().skip_flags;
                //uint32_t skip_undo_db = skip & skip_undo_block;

                if (!(skip & static_cast<uint32_t>(validation_steps::skip_fork_db))) {
                    shared_ptr<fork_item> new_head = _fork_db.push_block(new_block);
                    _maybe_warn_multiple_production(new_head->num);
                    //If the head block from the longest chain does not build off of the current head, we need to switch forks.
                    if (new_head->data.previous != head_block_id()) {
                        //If the newly pushed block is the same height as head, we get head back in new_head
                        //Only switch forks if new_head is actually higher than head
                        if (new_head->data.block_num() > head_block_num()) {
                            // wlog( "Switching to fork: ${id}", ("id",new_head->data.id()) );
                            auto branches = _fork_db.fetch_branch_from(new_head->data.id(), head_block_id());

                            // pop blocks until we hit the forked block
                            while (head_block_id() !=
                                   branches.second.back()->data.previous) {
                                pop_block();
                            }

                            // push all blocks on the new fork
                            for (auto ritr = branches.first.rbegin();
                                 ritr != branches.first.rend(); ++ritr) {
                                // ilog( "pushing blocks from fork ${n} ${id}", ("n",(*ritr)->data.block_num())("id",(*ritr)->data.id()) );
                                optional<fc::exception> except;
                                try {
                                    auto session = start_undo_session(true);
                                    apply_block((*ritr)->data, skip);
                                    session.push();
                                }
                                catch (const fc::exception &e) {
                                    except = e;
                                }
                                if (except) {
                                    // wlog( "exception thrown while switching forks ${e}", ("e",except->to_detail_string() ) );
                                    // remove the rest of branches.first from the fork_db, those blocks are invalid
                                    while (ritr != branches.first.rend()) {
                                        _fork_db.remove((*ritr)->data.id());
                                        ++ritr;
                                    }
                                    _fork_db.set_head(branches.second.front());

                                    // pop all blocks from the bad fork
                                    while (head_block_id() !=
                                           branches.second.back()->data.previous) {
                                        pop_block();
                                    }

                                    // restore all blocks from the good fork
                                    for (auto ritr = branches.second.rbegin();
                                         ritr !=
                                         branches.second.rend(); ++ritr) {
                                        auto session = start_undo_session(true);
                                        apply_block((*ritr)->data, skip);
                                        session.push();
                                    }
                                    throw *except;
                                }
                            }
                            return true;
                        } else {
                            return false;
                        }
                    }
                }

                try {
                    auto session = start_undo_session(true);
                    apply_block(new_block, skip);
                    session.push();
                }
                catch (const fc::exception &e) {
                    elog("Failed to push new block:\n${e}", ("e", e.to_detail_string()));
                    _fork_db.remove(new_block.id());
                    throw;
                }

                return false;
            } FC_CAPTURE_AND_RETHROW()
        }

/**
 * Attempts to push the transaction into the pending queue
 *
 * When called to push a locally generated transaction, set the skip_block_size_check bit on the skip argument. This
 * will allow the transaction to be pushed even if it causes the pending block size to exceed the maximum block size.
 * Although the transaction will probably not propagate further now, as the peers are likely to have their pending
 * queues full as well, it will be kept in the queue to be propagated later when a new block flushes out the pending
 * queues.
 */
        void database_basic::push_transaction(const signed_transaction &trx, uint32_t skip) {
            try {
                try {
                    FC_ASSERT(fc::raw::pack_size(trx) <= (get_dynamic_global_properties().maximum_block_size - 256));
                    set_producing(true);
                    detail::with_skip_flags(*this, skip,
                            [&]() {
                                with_write_lock([&]() {
                                    _push_transaction(trx);
                                });
                            });
                    set_producing(false);
                }
                catch (...) {
                    set_producing(false);
                    throw;
                }
            }
            FC_CAPTURE_AND_RETHROW((trx))
        }

        void database_basic::_push_transaction(const signed_transaction &trx) {
            // If this is the first transaction pushed after applying a block, start a new undo session.
            // This allows us to quickly rewind to the clean state of the head block, in case a new block arrives.
            if (!_pending_tx_session.valid()) {
                _pending_tx_session = start_undo_session(true);
            }

            // Create a temporary undo session as a child of _pending_tx_session.
            // The temporary session will be discarded by the destructor if
            // _apply_transaction fails.  If we make it to merge(), we
            // apply the changes.

            auto temp_session = start_undo_session(true);
            _apply_transaction(trx);
            _pending_tx.push_back(trx);

            notify_changed_objects();
            // The transaction applied successfully. Merge its changes into the pending block session.
            temp_session.squash();

            // notify anyone listening to pending transactions
            notify_on_pending_transaction(trx);
        }

        signed_block database_basic::generate_block(
                fc::time_point_sec when,
                const account_name_type &witness_owner,
                const fc::ecc::private_key &block_signing_private_key,
                uint32_t skip /* = 0 */
        ) {
            signed_block result;
            detail::with_skip_flags(*this, skip, [&]() {
                try {
                    result = _generate_block(when, witness_owner, block_signing_private_key);
                }
                FC_CAPTURE_AND_RETHROW((witness_owner))
            });
            return result;
        }


        signed_block database_basic::_generate_block(
                fc::time_point_sec when,
                const account_name_type &witness_owner,
                const fc::ecc::private_key &block_signing_private_key
        ) {
            uint32_t skip = get_node_properties().skip_flags;
            uint32_t slot_num = get_slot_at_time(when);
            FC_ASSERT(slot_num > 0);
            string scheduled_witness = get_scheduled_witness(*this,slot_num);
            FC_ASSERT(scheduled_witness == witness_owner);

            const auto &witness_obj = get_witness(*this,witness_owner);

            if (!(skip & static_cast<uint32_t >(validation_steps::skip_witness_signature)))
                FC_ASSERT(witness_obj.signing_key == block_signing_private_key.get_public_key());

            static const size_t max_block_header_size =
                    fc::raw::pack_size(signed_block_header()) + 4;
            auto maximum_block_size = get_dynamic_global_properties().maximum_block_size; //STEEMIT_MAX_BLOCK_SIZE;
            size_t total_block_size = max_block_header_size;

            signed_block pending_block;

            with_write_lock([&]() {
                //
                // The following code throws away existing pending_tx_session and
                // rebuilds it by re-applying pending transactions.
                //
                // This rebuild is necessary because pending transactions' validity
                // and semantics may have changed since they were received, because
                // time-based semantics are evaluated based on the current block
                // time.  These changes can only be reflected in the database_basic when
                // the value of the "when" variable is known, which means we need to
                // re-apply pending transactions in this method.
                //
                _pending_tx_session.reset();
                _pending_tx_session = start_undo_session(true);

                uint64_t postponed_tx_count = 0;
                // pop pending state (reset to head block state)
                for (const signed_transaction &tx : _pending_tx) {
                    // Only include transactions that have not expired yet for currently generating block,
                    // this should clear problem transactions and allow block production to continue

                    if (tx.expiration < when) {
                        continue;
                    }

                    uint64_t new_total_size =
                            total_block_size + fc::raw::pack_size(tx);

                    // postpone transaction if it would make block too big
                    if (new_total_size >= maximum_block_size) {
                        postponed_tx_count++;
                        continue;
                    }

                    try {
                        auto temp_session = start_undo_session(true);
                        _apply_transaction(tx);
                        temp_session.squash();

                        total_block_size += fc::raw::pack_size(tx);
                        pending_block.transactions.push_back(tx);
                    }
                    catch (const fc::exception &e) {
                        // Do nothing, transaction will not be re-applied
                        //wlog( "Transaction was not processed while generating block due to ${e}", ("e", e) );
                        //wlog( "The transaction was ${t}", ("t", tx) );
                    }
                }
                if (postponed_tx_count > 0) {
                    wlog("Postponed ${n} transactions due to block size limit", ("n", postponed_tx_count));
                }

                _pending_tx_session.reset();
            });

            // We have temporarily broken the invariant that
            // _pending_tx_session is the result of applying _pending_tx, as
            // _pending_tx now consists of the set of postponed transactions.
            // However, the push_block() call below will re-create the
            // _pending_tx_session.

            pending_block.previous = head_block_id();
            pending_block.timestamp = when;
            pending_block.transaction_merkle_root = pending_block.calculate_merkle_root();
            pending_block.witness = witness_owner;
            if (has_hardfork(STEEMIT_HARDFORK_0_5__54)) {
                const auto &witness = get_witness(*this,witness_owner);

                if (witness.running_version != STEEMIT_BLOCKCHAIN_VERSION) {
                    pending_block.extensions.insert(block_header_extensions(STEEMIT_BLOCKCHAIN_VERSION));
                }

                const auto &hfp = get_hardfork_property_object();

                if (hfp.current_hardfork_version <
                    STEEMIT_BLOCKCHAIN_HARDFORK_VERSION // Binary is newer hardfork than has been applied
                    && (witness.hardfork_version_vote !=
                        _hardfork_versions[hfp.last_hardfork + 1] ||
                        witness.hardfork_time_vote !=
                        _hardfork_times[hfp.last_hardfork +
                                        1])) // Witness vote does not match binary configuration
                {
                    // Make vote match binary configuration
                    pending_block.extensions.insert(block_header_extensions(hardfork_version_vote(_hardfork_versions[
                            hfp.last_hardfork + 1], _hardfork_times[
                            hfp.last_hardfork + 1])));
                } else if (hfp.current_hardfork_version ==
                           STEEMIT_BLOCKCHAIN_HARDFORK_VERSION // Binary does not know of a new hardfork
                           && witness.hardfork_version_vote >
                              STEEMIT_BLOCKCHAIN_HARDFORK_VERSION) // Voting for hardfork in the future, that we do not know of...
                {
                    // Make vote match binary configuration. This is vote to not apply the new hardfork.
                    pending_block.extensions.insert(block_header_extensions(hardfork_version_vote(_hardfork_versions[hfp.last_hardfork], _hardfork_times[hfp.last_hardfork])));
                }
            }

            if (!(skip & static_cast<uint32_t >(validation_steps::skip_witness_signature))) {
                pending_block.sign(block_signing_private_key);
            }

            // TODO:  Move this to _push_block() so session is restored.
            if (!(skip & static_cast<uint32_t >(validation_steps::skip_block_size_check))) {
                FC_ASSERT(fc::raw::pack_size(pending_block) <= STEEMIT_MAX_BLOCK_SIZE);
            }

            push_block(pending_block, skip);

            return pending_block;
        }

/**
 * Removes the most recent block from the database_basic and
 * undoes any changes it made.
 */
        void database_basic::pop_block() {
            try {
                _pending_tx_session.reset();
                auto head_id = head_block_id();

                /// save the head block so we can recover its transactions
                optional<signed_block> head_block = fetch_block_by_id(head_id);
                STEEMIT_ASSERT(head_block.valid(), pop_empty_chain, "there are no blocks to pop");

                _fork_db.pop_block();
                undo();

                _popped_tx.insert(_popped_tx.begin(), head_block->transactions.begin(), head_block->transactions.end());

            }
            FC_CAPTURE_AND_RETHROW()
        }

        void database_basic::clear_pending() {
            try {
                assert((_pending_tx.size() == 0) ||
                       _pending_tx_session.valid());
                _pending_tx.clear();
                _pending_tx_session.reset();
            }
            FC_CAPTURE_AND_RETHROW()
        }

        void database_basic::notify_pre_apply_operation(operation_notification &note) {
            note.trx_id = _current_trx_id;
            note.block = _current_block_num;
            note.trx_in_block = _current_trx_in_block;
            note.op_in_trx = _current_op_in_trx;

            STEEMIT_TRY_NOTIFY(pre_apply_operation, note)
        }

        void database_basic::notify_post_apply_operation(const operation_notification &note) {
            STEEMIT_TRY_NOTIFY(post_apply_operation, note)
        }

        void database_basic::push_virtual_operation(const operation &op, bool force) {
            if (!force) {
#if defined( STEEMIT_BUILD_LOW_MEMORY ) && !defined( STEEMIT_BUILD_TESTNET )
                return;
#endif
            }

            FC_ASSERT(is_virtual_operation(op));
            operation_notification note(op);
            notify_pre_apply_operation(note);
            notify_post_apply_operation(note);
        }

        void database_basic::notify_applied_block(const signed_block &block) {
            STEEMIT_TRY_NOTIFY(applied_block, block)
        }

        void database_basic::notify_on_pending_transaction(const signed_transaction &tx) {
            STEEMIT_TRY_NOTIFY(on_pending_transaction, tx)
        }

        void database_basic::notify_on_applied_transaction(const signed_transaction &tx) {
            STEEMIT_TRY_NOTIFY(on_applied_transaction, tx)
        }

        uint32_t database_basic::get_slot_at_time(fc::time_point_sec when) const {
            fc::time_point_sec first_slot_time = get_slot_time(1);
            if (when < first_slot_time) {
                return 0;
            }
            return (when - first_slot_time).to_seconds() /
                   STEEMIT_BLOCK_INTERVAL + 1;
        }

        fc::time_point_sec database_basic::get_slot_time(uint32_t slot_num) const {
            if (slot_num == 0) {
                return fc::time_point_sec();
            }

            auto interval = STEEMIT_BLOCK_INTERVAL;
            const dynamic_global_property_object &dpo = get_dynamic_global_properties();

            if (head_block_num() == 0) {
                // n.b. first block is at genesis_time plus one block interval
                fc::time_point_sec genesis_time = dpo.time;
                return genesis_time + slot_num * interval;
            }

            int64_t head_block_abs_slot = head_block_time().sec_since_epoch() / interval;
            fc::time_point_sec head_slot_time(head_block_abs_slot * interval);

            // "slot 0" is head_slot_time
            // "slot 1" is head_slot_time,
            //   plus maint interval if head block is a maint block
            //   plus block interval if head block is not a maint block
            return head_slot_time + (slot_num * interval);
        }

         node_property_object &database_basic::node_properties() {
            return _node_property_object;
        }

        void database_basic::set_custom_operation_interpreter(const std::string &id, std::shared_ptr<custom_operation_interpreter> registry) {
            bool inserted = _custom_operation_interpreters.emplace(id, registry).second;
            // This assert triggering means we're mis-configured (multiple registrations of custom JSON evaluator for same ID)
            FC_ASSERT(inserted);
        }

        std::shared_ptr<custom_operation_interpreter> database_basic::get_custom_json_evaluator(const std::string &id) {
            auto it = _custom_operation_interpreters.find(id);
            if (it != _custom_operation_interpreters.end()) {
                return it->second;
            }
            return std::shared_ptr<custom_operation_interpreter>();
        }


        const std::string &database_basic::get_json_schema() const {
            return _json_schema;
        }

        void database_basic::init_schema() {
            /*
           done_adding_indexes();

           db_schema ds;

           std::vector< std::shared_ptr< abstract_schema > > schema_list;

           std::vector< object_schema > object_schemas;
           get_object_schemas( object_schemas );

           for( const object_schema& oschema : object_schemas )
           {
              ds.object_types.emplace_back();
              ds.object_types.back().space_type.first = oschema.space_id;
              ds.object_types.back().space_type.second = oschema.type_id;
              oschema.schema->get_name( ds.object_types.back().type );
              schema_list.push_back( oschema.schema );
           }

           std::shared_ptr< abstract_schema > operation_schema = get_schema_for_type< operation >();
           operation_schema->get_name( ds.operation_type );
           schema_list.push_back( operation_schema );

           for( const std::pair< std::string, std::shared_ptr< custom_operation_interpreter > >& p : _custom_operation_interpreters )
           {
              ds.custom_operation_types.emplace_back();
              ds.custom_operation_types.back().id = p.first;
              schema_list.push_back( p.second->get_operation_schema() );
              schema_list.back()->get_name( ds.custom_operation_types.back().type );
           }

           graphene::db::add_dependent_schemas( schema_list );
           std::sort( schema_list.begin(), schema_list.end(),
              []( const std::shared_ptr< abstract_schema >& a,
                  const std::shared_ptr< abstract_schema >& b )
              {
                 return a->id < b->id;
              } );
           auto new_end = std::unique( schema_list.begin(), schema_list.end(),
              []( const std::shared_ptr< abstract_schema >& a,
                  const std::shared_ptr< abstract_schema >& b )
              {
                 return a->id == b->id;
              } );
           schema_list.erase( new_end, schema_list.end() );

           for( std::shared_ptr< abstract_schema >& s : schema_list )
           {
              std::string tname;
              s->get_name( tname );
              FC_ASSERT( ds.types.find( tname ) == ds.types.end(), "types with different ID's found for name ${tname}", ("tname", tname) );
              std::string ss;
              s->get_str_schema( ss );
              ds.types.emplace( tname, ss );
           }

           _json_schema = fc::json::to_string( ds );
           return;*/
        }

        void database_basic::init_genesis(uint64_t init_supply) {
            try {
                struct auth_inhibitor {
                    auth_inhibitor(database_basic &db)
                            : db(db),
                              old_flags(db.node_properties().skip_flags) {
                        db.node_properties().skip_flags |= static_cast<uint32_t >(validation_steps::skip_authority_check);
                    }

                    ~auth_inhibitor() {
                        db.node_properties().skip_flags = old_flags;
                    }

                private:
                    database_basic &db;
                    uint32_t old_flags;
                } inhibitor(*this);

                // Create blockchain accounts
                public_key_type init_public_key(STEEMIT_INIT_PUBLIC_KEY);

                create<account_object>([&](account_object &a) {
                    a.name = STEEMIT_MINER_ACCOUNT;
                });
                create<account_authority_object>([&](account_authority_object &auth) {
                    auth.account = STEEMIT_MINER_ACCOUNT;
                    auth.owner.weight_threshold = 1;
                    auth.active.weight_threshold = 1;
                });

                create<account_object>([&](account_object &a) {
                    a.name = STEEMIT_NULL_ACCOUNT;
                });
                create<account_authority_object>([&](account_authority_object &auth) {
                    auth.account = STEEMIT_NULL_ACCOUNT;
                    auth.owner.weight_threshold = 1;
                    auth.active.weight_threshold = 1;
                });

                create<account_object>([&](account_object &a) {
                    a.name = STEEMIT_TEMP_ACCOUNT;
                });
                create<account_authority_object>([&](account_authority_object &auth) {
                    auth.account = STEEMIT_TEMP_ACCOUNT;
                    auth.owner.weight_threshold = 0;
                    auth.active.weight_threshold = 0;
                });

                for (int i = 0; i < STEEMIT_NUM_INIT_MINERS; ++i) {
                    create<account_object>([&](account_object &a) {
                        a.name = STEEMIT_INIT_MINER_NAME +
                                 (i ? fc::to_string(i) : std::string());
                        a.memo_key = init_public_key;
                        a.balance = asset(i ? 0 : init_supply, STEEM_SYMBOL);
                    });

                    create<account_authority_object>([&](account_authority_object &auth) {
                        auth.account = STEEMIT_INIT_MINER_NAME + (i ? fc::to_string(i) : std::string());
                        auth.owner.add_authority(init_public_key, 1);
                        auth.owner.weight_threshold = 1;
                        auth.active = auth.owner;
                        auth.posting = auth.active;
                    });

                    create<witness_object>([&](witness_object &w) {
                        w.owner = STEEMIT_INIT_MINER_NAME + (i ? fc::to_string(i) : std::string());
                        w.signing_key = init_public_key;
                        w.schedule = witness_object::miner;
                    });
                }

                create<dynamic_global_property_object>([&](dynamic_global_property_object &p) {
                    p.current_witness = STEEMIT_INIT_MINER_NAME;
                    p.time = STEEMIT_GENESIS_TIME;
                    p.recent_slots_filled = fc::uint128::max_value();
                    p.participation_count = 128;
                    p.current_supply = asset(init_supply, STEEM_SYMBOL);
                    p.virtual_supply = p.current_supply;
                    p.maximum_block_size = STEEMIT_MAX_BLOCK_SIZE;
                });

                // Nothing to do
                create<feed_history_object>([&](feed_history_object &o) {});
                for (int i = 0; i < 0x10000; i++) {
                    create<block_summary_object>([&](block_summary_object &) {});
                }
                create<hardfork_property_object>([&](hardfork_property_object &hpo) {
                    hpo.processed_hardforks.push_back(STEEMIT_GENESIS_TIME);
                });

                // Create witness scheduler
                create<witness_schedule_object>([&](witness_schedule_object &wso) {
                    wso.current_shuffled_witnesses[0] = STEEMIT_INIT_MINER_NAME;
                });
            }
            FC_CAPTURE_AND_RETHROW()
        }


        void database_basic::validate_transaction(const signed_transaction &trx) {
            database_basic::with_write_lock([&]() {
                auto session = start_undo_session(true);
                _apply_transaction(trx);
                session.undo();
            });
        }

        void database_basic::notify_changed_objects() {
            try {
                /*vector< graphene::chainbase::generic_id > ids;
      get_changed_ids( ids );
      STEEMIT_TRY_NOTIFY( changed_objects, ids )*/
                /*
      if( _undo_db.enabled() )
      {
         const auto& head_undo = _undo_db.head();
         vector<object_id_type> changed_ids;  changed_ids.reserve(head_undo.old_values.size());
         for( const auto& item : head_undo.old_values ) changed_ids.push_back(item.first);
         for( const auto& item : head_undo.new_ids ) changed_ids.push_back(item);
         vector<const object*> removed;
         removed.reserve( head_undo.removed.size() );
         for( const auto& item : head_undo.removed )
         {
            changed_ids.push_back( item.first );
            removed.emplace_back( item.second.get() );
         }
         STEEMIT_TRY_NOTIFY( changed_objects, changed_ids )
      }
      */
            }
            FC_CAPTURE_AND_RETHROW()

        }

        void database_basic::set_flush_interval(uint32_t flush_blocks) {
            _flush_blocks = flush_blocks;
            _next_flush_block = 0;
        }

//////////////////// private methods ////////////////////

        void database_basic::apply_block(const signed_block &next_block, uint32_t skip) {
            try {
                //fc::time_point begin_time = fc::time_point::now();

                auto block_num = next_block.block_num();
                if (_checkpoints.size() &&
                    _checkpoints.rbegin()->second != block_id_type()) {
                    auto itr = _checkpoints.find(block_num);
                    if (itr != _checkpoints.end())
                        FC_ASSERT(next_block.id() ==
                                  itr->second, "Block did not match checkpoint", ("checkpoint", *itr)("block_id", next_block.id()));

                    if (_checkpoints.rbegin()->first >= block_num) {
                        skip = static_cast<uint32_t>(validation_steps::skip_witness_signature)
                               | static_cast<uint32_t>(validation_steps::skip_transaction_signatures)
                               | static_cast<uint32_t>(validation_steps::skip_transaction_dupe_check)
                               | static_cast<uint32_t>(validation_steps::skip_fork_db)
                               | static_cast<uint32_t>(validation_steps::skip_block_size_check)
                               | static_cast<uint32_t>(validation_steps::skip_tapos_check)
                               | static_cast<uint32_t>(validation_steps::skip_authority_check)
                               /* | skip_merkle_check While blockchain is being downloaded, txs need to be validated against block headers */
                               | static_cast<uint32_t>(validation_steps::skip_undo_history_check)
                               | static_cast<uint32_t>( validation_steps::skip_witness_schedule_check)
                               | static_cast<uint32_t>(validation_steps::skip_validate)
                               | static_cast<uint32_t>(validation_steps::skip_validate_invariants);
                    }
                }

                detail::with_skip_flags(*this, skip, [&]() {
                    _apply_block(next_block);
                });

                /*try
   {
   /// check invariants
   if( is_producing() || !( skip & skip_validate_invariants ) )
      validate_invariants();
   }
   FC_CAPTURE_AND_RETHROW( (next_block) );*/

                //fc::time_point end_time = fc::time_point::now();
                //fc::microseconds dt = end_time - begin_time;
                if (_flush_blocks != 0) {
                    if (_next_flush_block == 0) {
                        uint32_t lep = block_num + 1 + _flush_blocks * 9 / 10;
                        uint32_t rep = block_num + 1 + _flush_blocks;

                        // use time_point::now() as RNG source to pick block randomly between lep and rep
                        uint32_t span = rep - lep;
                        uint32_t x = lep;
                        if (span > 0) {
                            uint64_t now = uint64_t(fc::time_point::now().time_since_epoch().count());
                            x += now % span;
                        }
                        _next_flush_block = x;
//                        ilog("Next flush scheduled at block ${b}", ("b", x));
                    }

                    if (_next_flush_block == block_num) {
                        _next_flush_block = 0;
//                        ilog("Flushing database_basic shared memory at block ${b}", ("b", block_num));
                        chainbase::database::flush();
                    }
                }

                uint32_t free_gb = uint32_t(
                        get_free_memory() / (1024 * 1024 * 1024));
                if ((free_gb < _last_free_gb_printed) ||
                    (free_gb > _last_free_gb_printed + 1)) {
                    ilog("Free memory is now ${n}G", ("n", free_gb));
                    _last_free_gb_printed = free_gb;
                }

            } FC_CAPTURE_AND_RETHROW((next_block))
        }

        void database_basic::_apply_block(const signed_block &next_block) {
            try {
                uint32_t next_block_num = next_block.block_num();
                //block_id_type next_block_id = next_block.id();

                uint32_t skip = get_node_properties().skip_flags;

                if (!(skip & static_cast<uint32_t >(validation_steps::skip_merkle_check))) {
                    auto merkle_root = next_block.calculate_merkle_root();

                    try {
                        FC_ASSERT(next_block.transaction_merkle_root == merkle_root, "Merkle check failed", ("next_block.transaction_merkle_root", next_block.transaction_merkle_root)("calc", merkle_root)("next_block", next_block)("id", next_block.id()));
                    }
                    catch (fc::assert_exception &e) {
                        const auto &merkle_map = get_shared_db_merkle();
                        auto itr = merkle_map.find(next_block_num);

                        if (itr == merkle_map.end() ||
                            itr->second != merkle_root) {
                            throw e;
                        }
                    }
                }

                const witness_object &signing_witness = validate_block_header(skip, next_block);

                _current_block_num = next_block_num;
                _current_trx_in_block = 0;

                const auto &gprops = get_dynamic_global_properties();
                auto block_size = fc::raw::pack_size(next_block);
                if (has_hardfork(STEEMIT_HARDFORK_0_12)) {
                    FC_ASSERT(block_size <= gprops.maximum_block_size, "Block Size is too Big", ("next_block_num", next_block_num)("block_size", block_size)("max", gprops.maximum_block_size));
                }

                /// modify current witness so transaction evaluators can know who included the transaction,
                /// this is mostly for POW operations which must pay the current_witness
                modify(gprops, [&](dynamic_global_property_object &dgp) {
                    dgp.current_witness = next_block.witness;
                });

                /// parse witness version reporting
                process_header_extensions(next_block);

                if (has_hardfork(STEEMIT_HARDFORK_0_5__54)) // Cannot remove after hardfork
                {
                    const auto &witness = get_witness(*this,next_block.witness);
                    const auto &hardfork_state = get_hardfork_property_object();
                    FC_ASSERT(witness.running_version >=
                              hardfork_state.current_hardfork_version,
                            "Block produced by witness that is not running current hardfork",
                            ("witness", witness)("next_block.witness", next_block.witness)("hardfork_state", hardfork_state)
                    );
                }

                for (const auto &trx : next_block.transactions) {
                    /* We do not need to push the undo state for each transaction
       * because they either all apply and are valid or the
       * entire block fails to apply.  We only need an "undo" state
       * for transactions when validating broadcast transactions or
       * when building a block.
       */
                    apply_transaction(trx, skip);
                    ++_current_trx_in_block;
                }
                //TODO Big problem
/*
                update_global_dynamic_data(next_block);
                update_signing_witness(signing_witness, next_block);

                update_last_irreversible_block();

                create_block_summary(next_block);
                clear_expired_transactions();
                clear_expired_orders();
                clear_expired_delegations();
                update_witness_schedule(*this);

                update_median_feed();
                update_virtual_supply();

                clear_null_account_balance();
                process_funds();
                process_conversions();
                process_comment_cashout();
                process_vesting_withdrawals();
                process_savings_withdraws();
                pay_liquidity_reward();
                update_virtual_supply();

                account_recovery_processing();
                expire_escrow_ratification();
                process_decline_voting_rights();

                process_hardforks();
*/
                // notify observers that the block has been applied
                notify_applied_block(next_block);

                notify_changed_objects();
            } //FC_CAPTURE_AND_RETHROW( (next_block.block_num()) )  }
            FC_CAPTURE_LOG_AND_RETHROW((next_block.block_num()))
        }

        void database_basic::process_header_extensions(const signed_block &next_block) {
            auto itr = next_block.extensions.begin();

            while (itr != next_block.extensions.end()) {
                switch (itr->which()) {
                    case 0: // void_t
                        break;
                    case 1: // version
                    {
                        auto reported_version = itr->get<version>();
                        const auto &signing_witness = get_witness(*this,next_block.witness);
                        //idump( (next_block.witness)(signing_witness.running_version)(reported_version) );

                        if (reported_version !=
                            signing_witness.running_version) {
                            modify(signing_witness, [&](witness_object &wo) {
                                wo.running_version = reported_version;
                            });
                        }
                        break;
                    }
                    case 2: // hardfork_version vote
                    {
                        auto hfv = itr->get<hardfork_version_vote>();
                        const auto &signing_witness = get_witness(*this,next_block.witness);
                        //idump( (next_block.witness)(signing_witness.running_version)(hfv) );

                        if (hfv.hf_version !=
                            signing_witness.hardfork_version_vote ||
                            hfv.hf_time != signing_witness.hardfork_time_vote) {
                            modify(signing_witness, [&](witness_object &wo) {
                                wo.hardfork_version_vote = hfv.hf_version;
                                wo.hardfork_time_vote = hfv.hf_time;
                            });
                        }

                        break;
                    }
                    default:
                        FC_ASSERT(false, "Unknown extension in block header");
                }

                ++itr;
            }
        }


        void database_basic::apply_transaction(const signed_transaction &trx, uint32_t skip) {
            detail::with_skip_flags(*this, skip, [&]() { _apply_transaction(trx); });
            notify_on_applied_transaction(trx);
        }

        void database_basic::_apply_transaction(const signed_transaction &trx) {
            try {
                _current_trx_id = trx.id();
                uint32_t skip = get_node_properties().skip_flags;

                if (!(skip & static_cast<uint32_t>(validation_steps::skip_validate))) {   /* issue #505 explains why this skip_flag is disabled */
                    trx.validate();
                }

                auto &trx_idx = get_index<transaction_index>();
                const chain_id_type &chain_id = STEEMIT_CHAIN_ID;
                auto trx_id = trx.id();
                // idump((trx_id)(skip&skip_transaction_dupe_check));
                FC_ASSERT((skip & static_cast<uint32_t>(validation_steps::skip_transaction_dupe_check)) ||
                          trx_idx.indices().get<by_trx_id>().find(trx_id) ==
                          trx_idx.indices().get<by_trx_id>().end(),
                        "Duplicate transaction check failed", ("trx_ix", trx_id));

                if (!(skip & (static_cast<uint32_t>(validation_steps::skip_transaction_signatures) | static_cast<uint32_t >(validation_steps::skip_authority_check)))) {
                    auto get_active = [&](const string &name) { return authority(get<account_authority_object, by_account>(name).active); };
                    auto get_owner = [&](const string &name) { return authority(get<account_authority_object, by_account>(name).owner); };
                    auto get_posting = [&](const string &name) { return authority(get<account_authority_object, by_account>(name).posting); };

                    try {
                        trx.verify_authority(chain_id, get_active, get_owner, get_posting, STEEMIT_MAX_SIG_CHECK_DEPTH);
                    }
                    catch (protocol::tx_missing_active_auth &e) {
                        if (get_shared_db_merkle().find(head_block_num() + 1) ==
                            get_shared_db_merkle().end()) {
                            throw e;
                        }
                    }
                }
                flat_set<account_name_type> required;
                vector<authority> other;
                trx.get_required_authorities(required, required, required, other);

                auto trx_size = fc::raw::pack_size(trx);


                for (const auto &auth : required) {
                    const auto &acnt = get_account(*this,auth);

                    if (!has_hardfork(STEEMIT_HARDFORK_0_17__79)) {
                        dynamic_extension_worker().get("account")->invoke("old_update_account_bandwidth",acnt, trx_size, bandwidth_type::old_forum);
                    }

                    dynamic_extension_worker().get("account")->invoke("update_account_bandwidth",acnt, trx_size, bandwidth_type::forum);
                    for (const auto &op : trx.operations) {
                        if (is_market_operation(op)) {
                            if (!has_hardfork(STEEMIT_HARDFORK_0_17__79)) {
                                dynamic_extension_worker().get("account")->invoke("old_update_account_bandwidth",acnt, trx_size, bandwidth_type::old_market);
                            }

                            dynamic_extension_worker().get("account")->invoke("update_account_bandwidth",acnt, trx_size * 10, bandwidth_type::market);
                            break;
                        }
                    }
                }


                //Skip all manner of expiration and TaPoS checking if we're on block 1; It's impossible that the transaction is
                //expired, and TaPoS makes no sense as no blocks exist.
                if (BOOST_LIKELY(head_block_num() > 0)) {
                    if (!(skip & static_cast<uint32_t>(validation_steps::skip_tapos_check))) {
                        const auto &tapos_block_summary = get<block_summary_object>(trx.ref_block_num);
                        //Verify TaPoS block summary has correct ID prefix, and that this block's time is not past the expiration
                        FC_ASSERT(trx.ref_block_prefix ==
                                  tapos_block_summary.block_id._hash[1],
                                "", ("trx.ref_block_prefix", trx.ref_block_prefix)
                                ("tapos_block_summary", tapos_block_summary.block_id._hash[1]));
                    }

                    fc::time_point_sec now = head_block_time();

                    FC_ASSERT(trx.expiration <= now +
                                                fc::seconds(STEEMIT_MAX_TIME_UNTIL_EXPIRATION), "",
                            ("trx.expiration", trx.expiration)("now", now)("max_til_exp", STEEMIT_MAX_TIME_UNTIL_EXPIRATION));
                    if (is_producing() || has_hardfork(STEEMIT_HARDFORK_0_9)) // Simple solution to pending trx bug when now == trx.expiration
                        FC_ASSERT(now < trx.expiration, "", ("now", now)("trx.exp", trx.expiration));
                    FC_ASSERT(now <= trx.expiration, "", ("now", now)("trx.exp", trx.expiration));
                }

                //Insert transaction into unique transactions database_basic.
                if (!(skip & static_cast<uint32_t>(validation_steps::skip_transaction_dupe_check))) {
                    create<transaction_object>([&](transaction_object &transaction) {
                        transaction.trx_id = trx_id;
                        transaction.expiration = trx.expiration;
                        fc::raw::pack(transaction.packed_trx, trx);
                    });
                }

                //Finally process the operations
                _current_op_in_trx = 0;
                for (const auto &op : trx.operations) {
                    try {
                        apply_operation(op);
                        ++_current_op_in_trx;
                    } FC_CAPTURE_AND_RETHROW((op));
                }
                _current_trx_id = transaction_id_type();

            } FC_CAPTURE_AND_RETHROW((trx))
        }

        const witness_object &database_basic::validate_block_header(uint32_t skip, const signed_block &next_block) const {
            try {
                FC_ASSERT(head_block_id() == next_block.previous, "", ("head_block_id", head_block_id())("next.prev", next_block.previous));
                FC_ASSERT(head_block_time() < next_block.timestamp, "", ("head_block_time", head_block_time())("next", next_block.timestamp)("blocknum", next_block.block_num()));
                const witness_object &witness = get_witness(const_cast<database_basic&>(*this),next_block.witness);

                if (!(skip & static_cast<uint32_t >(validation_steps::skip_witness_signature)))
                    FC_ASSERT(next_block.validate_signee(witness.signing_key));

                if (!(skip & static_cast<uint32_t >(validation_steps::skip_witness_schedule_check))) {
                    uint32_t slot_num = get_slot_at_time(next_block.timestamp);
                    FC_ASSERT(slot_num > 0);

                    string scheduled_witness = get_scheduled_witness(const_cast<database_basic&>(*this),slot_num);

                    FC_ASSERT(witness.owner ==
                              scheduled_witness, "Witness produced block at wrong time",
                            ("block witness", next_block.witness)("scheduled", scheduled_witness)("slot_num", slot_num));
                }

                return witness;
            } FC_CAPTURE_AND_RETHROW()
        }

        void database_basic::create_block_summary(const signed_block &next_block) {
            try {
                block_summary_id_type sid(next_block.block_num() & 0xffff);
                modify(get<block_summary_object>(sid), [&](block_summary_object &p) {
                    p.block_id = next_block.id();
                });
            } FC_CAPTURE_AND_RETHROW()
        }




        void database_basic::update_last_irreversible_block() {
            try {
                const dynamic_global_property_object &dpo = get_dynamic_global_properties();

                /**
    * Prior to voting taking over, we must be more conservative...
    *
    */
                if (head_block_num() < STEEMIT_START_MINER_VOTING_BLOCK) {
                    modify(dpo, [&](dynamic_global_property_object &_dpo) {
                        if (head_block_num() > STEEMIT_MAX_WITNESSES) {
                            _dpo.last_irreversible_block_num = head_block_num() - STEEMIT_MAX_WITNESSES;
                        }
                    });
                } else {
                    const witness_schedule_object &wso = get_witness_schedule_object(*this);

                    vector<const witness_object *> wit_objs;
                    wit_objs.reserve(wso.num_scheduled_witnesses);
                    for (int i = 0; i < wso.num_scheduled_witnesses; i++) {
                        wit_objs.push_back(&get_witness(*this,wso.current_shuffled_witnesses[i]));
                    }

                    static_assert(STEEMIT_IRREVERSIBLE_THRESHOLD > 0, "irreversible threshold must be nonzero");

                    // 1 1 1 2 2 2 2 2 2 2 -> 2     .7*10 = 7
                    // 1 1 1 1 1 1 1 2 2 2 -> 1
                    // 3 3 3 3 3 3 3 3 3 3 -> 3

                    size_t offset = ((STEEMIT_100_PERCENT -
                                      STEEMIT_IRREVERSIBLE_THRESHOLD) *
                                     wit_objs.size() / STEEMIT_100_PERCENT);

                    std::nth_element(wit_objs.begin(),
                            wit_objs.begin() + offset, wit_objs.end(),
                            [](const witness_object *a, const witness_object *b) {
                                return a->last_confirmed_block_num <
                                       b->last_confirmed_block_num;
                            });

                    uint32_t new_last_irreversible_block_num = wit_objs[offset]->last_confirmed_block_num;

                    if (new_last_irreversible_block_num >
                        dpo.last_irreversible_block_num) {
                        modify(dpo, [&](dynamic_global_property_object &_dpo) {
                            _dpo.last_irreversible_block_num = new_last_irreversible_block_num;
                        });
                    }
                }

                commit(dpo.last_irreversible_block_num);

                if (!(get_node_properties().skip_flags & static_cast<uint32_t >(validation_steps::skip_block_log))) {
                    // output to block log based on new last irreverisible block num
                    const auto &tmp_head = _block_log.head();
                    uint64_t log_head_num = 0;

                    if (tmp_head) {
                        log_head_num = tmp_head->block_num();
                    }

                    if (log_head_num < dpo.last_irreversible_block_num) {
                        while (log_head_num < dpo.last_irreversible_block_num) {
                            std::shared_ptr<fork_item> block = _fork_db.fetch_block_on_main_branch_by_number(log_head_num + 1);
                            FC_ASSERT(block, "Current fork in the fork database_basic does not contain the last_irreversible_block");
                            _block_log.append(block->data);
                            log_head_num++;
                        }

                        _block_log.flush();
                    }
                }

                _fork_db.set_max_size(dpo.head_block_number -
                                      dpo.last_irreversible_block_num + 1);
            } FC_CAPTURE_AND_RETHROW()
        }

        void database_basic::clear_expired_transactions() {
            //Look for expired transactions in the deduplication list, and remove them.
            //Transactions must have expired by at least two forking windows in order to be removed.
            auto &transaction_idx = get_index<transaction_index>();
            const auto &dedupe_index = transaction_idx.indices().get<by_expiration>();
            while ((!dedupe_index.empty()) && (head_block_time() > dedupe_index.begin()->expiration)) {
                remove(*dedupe_index.begin());
            }
        }
////Todo Nex Refactoring

        const hardfork_property_object &database_basic::get_hardfork_property_object() const {
            try {
                return get<hardfork_property_object>();
            } FC_CAPTURE_AND_RETHROW()
        }

        void database_basic::process_hardforks() {
            try {
                // If there are upcoming hardforks and the next one is later, do nothing
                const auto &hardforks = get_hardfork_property_object();

                if (has_hardfork(STEEMIT_HARDFORK_0_5__54)) {
                    while (
                            _hardfork_versions[hardforks.last_hardfork] < hardforks.next_hardfork
                            &&
                            hardforks.next_hardfork_time <= head_block_time()
                            ) {
                        if (hardforks.last_hardfork < STEEMIT_NUM_HARDFORKS) {
                            apply_hardfork(hardforks.last_hardfork + 1);
                        } else {
                            throw unknown_hardfork_exception();
                        }
                    }
                } else {
                    while (hardforks.last_hardfork < STEEMIT_NUM_HARDFORKS
                           &&
                           _hardfork_times[hardforks.last_hardfork + 1] <= head_block_time()
                           &&
                           hardforks.last_hardfork < STEEMIT_HARDFORK_0_5__54) {
                        apply_hardfork(hardforks.last_hardfork + 1);
                    }
                }
            }
            FC_CAPTURE_AND_RETHROW()
        }

        bool database_basic::has_hardfork(uint32_t hardfork) const {
            return get_hardfork_property_object().processed_hardforks.size() >
                   hardfork;
        }

        void database_basic::set_hardfork(uint32_t hardfork, bool apply_now) {
            auto const &hardforks = get_hardfork_property_object();

            for (uint32_t i = hardforks.last_hardfork + 1; i <= hardfork && i <= STEEMIT_NUM_HARDFORKS; i++) {
                if (i <= STEEMIT_HARDFORK_0_5__54) {
                    _hardfork_times[i] = head_block_time();
                } else {
                    modify(hardforks, [&](hardfork_property_object &hpo) {
                        hpo.next_hardfork = _hardfork_versions[i];
                        hpo.next_hardfork_time = head_block_time();
                    });
                }

                if (apply_now) {
                    apply_hardfork(i);
                }
            }
        }

        void database_basic::apply_hardfork(uint32_t hardfork) {

            //TODO Big problem
            /*
            if (_log_hardforks) {
                elog("HARDFORK ${hf} at block ${b}", ("hf", hardfork)("b", head_block_num()));
            }

            switch (hardfork) {
                case STEEMIT_HARDFORK_0_1:
                    perform_vesting_share_split(10000);
#ifdef STEEMIT_BUILD_TESTNET
                {
                            custom_operation test_op;
                            string op_msg = "Testnet: Hardfork applied";
                            test_op.data = vector<char>(op_msg.begin(), op_msg.end());
                            test_op.required_auths.insert(STEEMIT_INIT_MINER_NAME);
                            operation op = test_op;   // we need the operation object to live to the end of this scope
                            operation_notification note(op);
                            notify_pre_apply_operation(note);
                            notify_post_apply_operation(note);
                }
                break;
#endif
                    break;
                case STEEMIT_HARDFORK_0_2:
                    retally_witness_votes();
                    break;
                case STEEMIT_HARDFORK_0_3:
                    retally_witness_votes();
                    break;
                case STEEMIT_HARDFORK_0_4:
                    reset_virtual_schedule_time();
                    break;
                case STEEMIT_HARDFORK_0_5:
                    break;
                case STEEMIT_HARDFORK_0_6:
                    retally_witness_vote_counts();
                    retally_comment_children();
                    break;
                case STEEMIT_HARDFORK_0_7:
                    break;
                case STEEMIT_HARDFORK_0_8:
                    retally_witness_vote_counts(true);
                    break;
                case STEEMIT_HARDFORK_0_9: {

                }
                    break;
                case STEEMIT_HARDFORK_0_10:
                    retally_liquidity_weight();
                    break;
                case STEEMIT_HARDFORK_0_11:
                    break;
                case STEEMIT_HARDFORK_0_12: {
                    const auto &comment_idx = get_index<comment_index>().indices();

                    for (auto itr = comment_idx.begin(); itr != comment_idx.end(); ++itr) {
                        // At the hardfork time, all new posts with no votes get their cashout time set to +12 hrs from head block time.
                        // All posts with a payout get their cashout time set to +30 days. This hardfork takes place within 30 days
                        // initial payout so we don't have to handle the case of posts that should be frozen that aren't
                        if (itr->parent_author == STEEMIT_ROOT_POST_PARENT) {
                            // Post has not been paid out and has no votes (cashout_time == 0 === net_rshares == 0, under current semmantics)
                            if (itr->last_payout == fc::time_point_sec::min() &&
                                itr->cashout_time ==
                                fc::time_point_sec::maximum()) {
                                modify(*itr, [&](comment_object &c) {
                                    c.cashout_time = head_block_time() + STEEMIT_CASHOUT_WINDOW_SECONDS_PRE_HF17;
                                });
                            }
                                // Has been paid out, needs to be on second cashout window
                            else if (itr->last_payout > fc::time_point_sec()) {
                                modify(*itr, [&](comment_object &c) {
                                    c.cashout_time = c.last_payout + STEEMIT_SECOND_CASHOUT_WINDOW;
                                });
                            }
                        }
                    }

                    modify(get<account_authority_object, by_account>(STEEMIT_MINER_ACCOUNT),
                                      [&](account_authority_object &auth) {
                                          auth.posting = authority();
                                          auth.posting.weight_threshold = 1;
                                      });

                    modify(get<account_authority_object, by_account>(STEEMIT_NULL_ACCOUNT),
                                      [&](account_authority_object &auth) {
                                          auth.posting = authority();
                                          auth.posting.weight_threshold = 1;
                                      });

                    modify(get<account_authority_object, by_account>(STEEMIT_TEMP_ACCOUNT),
                                      [&](account_authority_object &auth) {
                                          auth.posting = authority();
                                          auth.posting.weight_threshold = 1;
                                      });
                }
                    break;
                case STEEMIT_HARDFORK_0_13:
                    break;
                case STEEMIT_HARDFORK_0_14:
                    break;
                case STEEMIT_HARDFORK_0_15:
                    break;
                case STEEMIT_HARDFORK_0_16: {
                    modify(get_feed_history(), [&](feed_history_object &fho) {
                        while (fho.price_history.size() > STEEMIT_FEED_HISTORY_WINDOW) {
                            fho.price_history.pop_front();
                        }
                    });

                    for (const std::string &acc : hardfork16::get_compromised_accounts()) {
                        const account_object *account = find_account(acc);
                        if (account == nullptr) {
                            continue;
                        }

                        update_owner_authority(*account, authority(1, public_key_type(
                                "GLS8hLtc7rC59Ed7uNVVTXtF578pJKQwMfdTvuzYLwUi8GkNTh5F6"), 1));

                        modify(get<account_authority_object, by_account>(account->name),
                                          [&](account_authority_object &auth) {
                                              auth.active = authority(1, public_key_type(
                                                      "GLS8hLtc7rC59Ed7uNVVTXtF578pJKQwMfdTvuzYLwUi8GkNTh5F6"), 1);
                                              auth.posting = authority(1, public_key_type(
                                                      "GLS8hLtc7rC59Ed7uNVVTXtF578pJKQwMfdTvuzYLwUi8GkNTh5F6"), 1);
                                          });
                    }

                    create<reward_fund_object>([&](reward_fund_object &rfo) {
                        rfo.name = STEEMIT_POST_REWARD_FUND_NAME;
                        rfo.last_update = head_block_time();
                        rfo.percent_content_rewards = 0;
                        rfo.content_constant = utilities::get_content_constant_s().to_uint64();
                    });

                    create<reward_fund_object>([&](reward_fund_object &rfo) {
                        rfo.name = STEEMIT_COMMENT_REWARD_FUND_NAME;
                        rfo.last_update = head_block_time();
                        rfo.percent_content_rewards = 0;
                        rfo.content_constant = utilities::get_content_constant_s().to_uint64();
                    });
                }
                    break;

                case STEEMIT_HARDFORK_0_17: {
                    const auto &gpo = get_dynamic_global_properties();
                    auto reward_steem = gpo.total_reward_fund_steem;


                    modify(get<reward_fund_object, by_name>(STEEMIT_POST_REWARD_FUND_NAME),
                                      [&](reward_fund_object &rfo) {
                                          rfo.percent_content_rewards = STEEMIT_POST_REWARD_FUND_PERCENT;
                                          rfo.reward_balance = asset(
                                                  (reward_steem.amount.value * rfo.percent_content_rewards) /
                                                  STEEMIT_100_PERCENT, STEEM_SYMBOL);
                                          reward_steem -= rfo.reward_balance;

                                      });

                    modify(get<reward_fund_object, by_name>(STEEMIT_COMMENT_REWARD_FUND_NAME),
                                      [&](reward_fund_object &rfo) {
                                          rfo.percent_content_rewards = STEEMIT_COMMENT_REWARD_FUND_PERCENT;
                                          rfo.reward_balance = reward_steem;
                                      });

                    modify(gpo, [&](dynamic_global_property_object &g) {
                        g.total_reward_fund_steem = asset(0, STEEM_SYMBOL);
                        g.total_reward_shares2 = 0;

                    });

                    /*
                     * For all current comments we will either keep their current cashout time, or extend it to 1 week
                     * after creation.
                     *
                     * We cannot do a simple iteration by cashout time because we are editting cashout time.
                     * More specifically, we will be adding an explicit cashout time to all comments with parents.
                     * To find all discussions that have not been paid out we fir iterate over posts by cashout time.
                     * Before the hardfork these are all root posts. Iterate over all of their children, adding each
                     * to a specific list. Next, update payout times for all discussions on the root post. This defines
                     * the min cashout time for each child in the discussion. Then iterate over the children and set
                     * their cashout time in a similar way, grabbing the root post as their inherent cashout time.
                     */
            /*
                    const auto &comment_idx = get_index<comment_index, by_cashout_time>();
                    const auto &by_root_idx = get_index<comment_index, by_root>();
                    vector<const comment_object *> root_posts;
                    root_posts.reserve(60000);
                    vector<const comment_object *> replies;
                    replies.reserve(100000);

                    for (auto itr = comment_idx.begin();
                         itr != comment_idx.end() && itr->cashout_time <
                                                     fc::time_point_sec::maximum(); ++itr) {
                        root_posts.push_back(&(*itr));

                        for (auto reply_itr = by_root_idx.lower_bound(itr->id);
                             reply_itr != by_root_idx.end() &&
                             reply_itr->root_comment == itr->id; ++reply_itr) {
                            replies.push_back(&(*reply_itr));
                        }
                    }

                    for (auto itr : root_posts) {
                        modify(*itr, [&](comment_object &c) {
                            c.cashout_time = std::max(c.created +
                                                      STEEMIT_CASHOUT_WINDOW_SECONDS, c.cashout_time);
                            c.children_rshares2 = 0;
                        });
                    }

                    for (auto itr : replies) {
                        modify(*itr, [&](comment_object &c) {
                            c.cashout_time = std::max(calculate_discussion_payout_time(c),
                                                      c.created + STEEMIT_CASHOUT_WINDOW_SECONDS);
                            c.children_rshares2 = 0;
                        });
                    }
                }
                    break;

                default:
                    break;
            }

            modify(get_hardfork_property_object(), [&](hardfork_property_object &hfp) {
                FC_ASSERT(hardfork == hfp.last_hardfork + 1, "Hardfork being applied out of order", ("hardfork", hardfork)("hfp.last_hardfork", hfp.last_hardfork));
                FC_ASSERT(hfp.processed_hardforks.size() == hardfork, "Hardfork being applied out of order");
                hfp.processed_hardforks.push_back(_hardfork_times[hardfork]);
                hfp.last_hardfork = hardfork;
                hfp.current_hardfork_version = _hardfork_versions[hardfork];
                FC_ASSERT(hfp.processed_hardforks[hfp.last_hardfork] == _hardfork_times[hfp.last_hardfork], "Hardfork processing failed sanity check...");
            });

            push_virtual_operation(hardfork_operation(hardfork), true);
            */
        }

        void database_basic::init_hardforks() {
            _hardfork_times[0] = fc::time_point_sec(STEEMIT_GENESIS_TIME);
            _hardfork_versions[0] = hardfork_version(0, 0);
            FC_ASSERT(STEEMIT_HARDFORK_0_1 == 1, "Invalid hardfork configuration");
            _hardfork_times[STEEMIT_HARDFORK_0_1] = fc::time_point_sec(STEEMIT_HARDFORK_0_1_TIME);
            _hardfork_versions[STEEMIT_HARDFORK_0_1] = STEEMIT_HARDFORK_0_1_VERSION;
            FC_ASSERT(STEEMIT_HARDFORK_0_2 == 2, "Invlaid hardfork configuration");
            _hardfork_times[STEEMIT_HARDFORK_0_2] = fc::time_point_sec(STEEMIT_HARDFORK_0_2_TIME);
            _hardfork_versions[STEEMIT_HARDFORK_0_2] = STEEMIT_HARDFORK_0_2_VERSION;
            FC_ASSERT(STEEMIT_HARDFORK_0_3 == 3, "Invalid hardfork configuration");
            _hardfork_times[STEEMIT_HARDFORK_0_3] = fc::time_point_sec(STEEMIT_HARDFORK_0_3_TIME);
            _hardfork_versions[STEEMIT_HARDFORK_0_3] = STEEMIT_HARDFORK_0_3_VERSION;
            FC_ASSERT(STEEMIT_HARDFORK_0_4 == 4, "Invalid hardfork configuration");
            _hardfork_times[STEEMIT_HARDFORK_0_4] = fc::time_point_sec(STEEMIT_HARDFORK_0_4_TIME);
            _hardfork_versions[STEEMIT_HARDFORK_0_4] = STEEMIT_HARDFORK_0_4_VERSION;
            FC_ASSERT(STEEMIT_HARDFORK_0_5 == 5, "Invalid hardfork configuration");
            _hardfork_times[STEEMIT_HARDFORK_0_5] = fc::time_point_sec(STEEMIT_HARDFORK_0_5_TIME);
            _hardfork_versions[STEEMIT_HARDFORK_0_5] = STEEMIT_HARDFORK_0_5_VERSION;
            FC_ASSERT(STEEMIT_HARDFORK_0_6 == 6, "Invalid hardfork configuration");
            _hardfork_times[STEEMIT_HARDFORK_0_6] = fc::time_point_sec(STEEMIT_HARDFORK_0_6_TIME);
            _hardfork_versions[STEEMIT_HARDFORK_0_6] = STEEMIT_HARDFORK_0_6_VERSION;
            FC_ASSERT(STEEMIT_HARDFORK_0_7 == 7, "Invalid hardfork configuration");
            _hardfork_times[STEEMIT_HARDFORK_0_7] = fc::time_point_sec(STEEMIT_HARDFORK_0_7_TIME);
            _hardfork_versions[STEEMIT_HARDFORK_0_7] = STEEMIT_HARDFORK_0_7_VERSION;
            FC_ASSERT(STEEMIT_HARDFORK_0_8 == 8, "Invalid hardfork configuration");
            _hardfork_times[STEEMIT_HARDFORK_0_8] = fc::time_point_sec(STEEMIT_HARDFORK_0_8_TIME);
            _hardfork_versions[STEEMIT_HARDFORK_0_8] = STEEMIT_HARDFORK_0_8_VERSION;
            FC_ASSERT(STEEMIT_HARDFORK_0_9 == 9, "Invalid hardfork configuration");
            _hardfork_times[STEEMIT_HARDFORK_0_9] = fc::time_point_sec(STEEMIT_HARDFORK_0_9_TIME);
            _hardfork_versions[STEEMIT_HARDFORK_0_9] = STEEMIT_HARDFORK_0_9_VERSION;
            FC_ASSERT(STEEMIT_HARDFORK_0_10 == 10, "Invalid hardfork configuration");
            _hardfork_times[STEEMIT_HARDFORK_0_10] = fc::time_point_sec(STEEMIT_HARDFORK_0_10_TIME);
            _hardfork_versions[STEEMIT_HARDFORK_0_10] = STEEMIT_HARDFORK_0_10_VERSION;
            FC_ASSERT(STEEMIT_HARDFORK_0_11 == 11, "Invalid hardfork configuration");
            _hardfork_times[STEEMIT_HARDFORK_0_11] = fc::time_point_sec(STEEMIT_HARDFORK_0_11_TIME);
            _hardfork_versions[STEEMIT_HARDFORK_0_11] = STEEMIT_HARDFORK_0_11_VERSION;
            FC_ASSERT(STEEMIT_HARDFORK_0_12 == 12, "Invalid hardfork configuration");
            _hardfork_times[STEEMIT_HARDFORK_0_12] = fc::time_point_sec(STEEMIT_HARDFORK_0_12_TIME);
            _hardfork_versions[STEEMIT_HARDFORK_0_12] = STEEMIT_HARDFORK_0_12_VERSION;
            FC_ASSERT(STEEMIT_HARDFORK_0_13 == 13, "Invalid hardfork configuration");
            _hardfork_times[STEEMIT_HARDFORK_0_13] = fc::time_point_sec(STEEMIT_HARDFORK_0_13_TIME);
            _hardfork_versions[STEEMIT_HARDFORK_0_13] = STEEMIT_HARDFORK_0_13_VERSION;
            FC_ASSERT(STEEMIT_HARDFORK_0_14 == 14, "Invalid hardfork configuration");
            _hardfork_times[STEEMIT_HARDFORK_0_14] = fc::time_point_sec(STEEMIT_HARDFORK_0_14_TIME);
            _hardfork_versions[STEEMIT_HARDFORK_0_14] = STEEMIT_HARDFORK_0_14_VERSION;
            FC_ASSERT(STEEMIT_HARDFORK_0_15 == 15, "Invalid hardfork configuration");
            _hardfork_times[STEEMIT_HARDFORK_0_15] = fc::time_point_sec(STEEMIT_HARDFORK_0_15_TIME);
            _hardfork_versions[STEEMIT_HARDFORK_0_15] = STEEMIT_HARDFORK_0_15_VERSION;
            FC_ASSERT(STEEMIT_HARDFORK_0_16 == 16, "Invalid hardfork configuration");
            _hardfork_times[STEEMIT_HARDFORK_0_16] = fc::time_point_sec(STEEMIT_HARDFORK_0_16_TIME);
            _hardfork_versions[STEEMIT_HARDFORK_0_16] = STEEMIT_HARDFORK_0_16_VERSION;
            FC_ASSERT(STEEMIT_HARDFORK_0_17 == 17, "Invalid hardfork configuration");
            _hardfork_times[STEEMIT_HARDFORK_0_17] = fc::time_point_sec(STEEMIT_HARDFORK_0_17_TIME);
            _hardfork_versions[STEEMIT_HARDFORK_0_17] = STEEMIT_HARDFORK_0_17_VERSION;

            const auto &hardforks = get_hardfork_property_object();
            FC_ASSERT(hardforks.last_hardfork <= STEEMIT_NUM_HARDFORKS, "Chain knows of more hardforks than configuration", ("hardforks.last_hardfork", hardforks.last_hardfork)("STEEMIT_NUM_HARDFORKS", STEEMIT_NUM_HARDFORKS));
            FC_ASSERT(_hardfork_versions[hardforks.last_hardfork] <= STEEMIT_BLOCKCHAIN_VERSION, "Blockchain version is older than last applied hardfork");
            FC_ASSERT(STEEMIT_BLOCKCHAIN_HARDFORK_VERSION == _hardfork_versions[STEEMIT_NUM_HARDFORKS]);
        }





        time_point_sec database_basic::head_block_time() const {
            return get_dynamic_global_properties().time;
        }

        void database_basic::update_global_dynamic_data(const signed_block &b) {
            try {
                auto block_size = fc::raw::pack_size(b);
                const dynamic_global_property_object &_dgp =
                        get_dynamic_global_properties();

                uint32_t missed_blocks = 0;
                if (head_block_time() != fc::time_point_sec()) {
                    missed_blocks = get_slot_at_time(b.timestamp);
                    assert(missed_blocks != 0);
                    missed_blocks--;
                    for (uint32_t i = 0; i < missed_blocks; ++i) {
                        const auto &witness_missed = get_witness(const_cast<database_basic&>(*this),get_scheduled_witness(const_cast<database_basic&>(*this),i + 1));
                        if (witness_missed.owner != b.witness) {
                            modify(witness_missed, [&](witness_object &w) {
                                w.total_missed++;
                                if (has_hardfork(STEEMIT_HARDFORK_0_14__278)) {
                                    if (head_block_num() - w.last_confirmed_block_num > STEEMIT_BLOCKS_PER_DAY) {
                                        w.signing_key = public_key_type();
                                        push_virtual_operation(shutdown_witness_operation(w.owner));
                                    }
                                }
                            });
                        }
                    }
                }

                // dynamic global properties updating
                modify(_dgp, [&](dynamic_global_property_object &dgp) {
                    // This is constant time assuming 100% participation. It is O(B) otherwise (B = Num blocks between update)
                    for (uint32_t i = 0; i < missed_blocks + 1; i++) {
                        dgp.participation_count -= dgp.recent_slots_filled.hi & 0x8000000000000000ULL ? 1 : 0;
                        dgp.recent_slots_filled = (dgp.recent_slots_filled << 1) + (i == 0 ? 1 : 0);
                        dgp.participation_count += (i == 0 ? 1 : 0);
                    }

                    dgp.head_block_number = b.block_num();
                    dgp.head_block_id = b.id();
                    dgp.time = b.timestamp;
                    dgp.current_aslot += missed_blocks + 1;
                    dgp.average_block_size = (99 * dgp.average_block_size + block_size) / 100;

                    /**
       *  About once per minute the average network use is consulted and used to
       *  adjust the reserve ratio. Anything above 50% usage reduces the ratio by
       *  half which should instantly bring the network from 50% to 25% use unless
       *  the demand comes from users who have surplus capacity. In other words,
       *  a 50% reduction in reserve ratio does not result in a 50% reduction in usage,
       *  it will only impact users who where attempting to use more than 50% of their
       *  capacity.
       *
       *  When the reserve ratio is at its max (10,000) a 50% reduction will take 3 to
       *  4 days to return back to maximum.  When it is at its minimum it will return
       *  back to its prior level in just a few minutes.
       *
       *  If the network reserve ratio falls under 100 then it is probably time to
       *  increase the capacity of the network.
       */
                    if (dgp.head_block_number % 20 == 0) {
                        if ((!has_hardfork(STEEMIT_HARDFORK_0_12__179) &&
                             dgp.average_block_size >
                             dgp.maximum_block_size / 2) ||
                            (has_hardfork(STEEMIT_HARDFORK_0_12__179) &&
                             dgp.average_block_size >
                             dgp.maximum_block_size / 4)) {
                            dgp.current_reserve_ratio /= 2; /// exponential back up
                        } else { /// linear growth... not much fine grain control near full capacity
                            dgp.current_reserve_ratio++;
                        }

                        if (has_hardfork(STEEMIT_HARDFORK_0_2) &&
                            dgp.current_reserve_ratio >
                            STEEMIT_MAX_RESERVE_RATIO) {
                            dgp.current_reserve_ratio = STEEMIT_MAX_RESERVE_RATIO;
                        }
                    }
                    dgp.max_virtual_bandwidth = (dgp.maximum_block_size *
                                                 dgp.current_reserve_ratio *
                                                 STEEMIT_BANDWIDTH_PRECISION *
                                                 STEEMIT_BANDWIDTH_AVERAGE_WINDOW_SECONDS) /
                                                STEEMIT_BLOCK_INTERVAL;
                });

                if (!(get_node_properties().skip_flags & static_cast<uint32_t >(validation_steps::skip_undo_history_check))) {
                    STEEMIT_ASSERT(_dgp.head_block_number -
                                   _dgp.last_irreversible_block_num <
                                   STEEMIT_MAX_UNDO_HISTORY, undo_database_exception,
                                   "The database does not have enough undo history to support a blockchain with so many missed blocks. "
                                           "Please add a checkpoint if you would like to continue applying blocks beyond this point.",
                                   ("last_irreversible_block_num", _dgp.last_irreversible_block_num)("head", _dgp.head_block_number)
                                           ("max_undo", STEEMIT_MAX_UNDO_HISTORY));
                }
            } FC_CAPTURE_AND_RETHROW()
        }

        void database_basic::update_virtual_supply() {
            try {
                modify(get_dynamic_global_properties(), [&](dynamic_global_property_object &dgp) {
                    dgp.virtual_supply = dgp.current_supply
                                         +
                                         (get_feed_history().current_median_history.is_null()
                                          ? asset(0, STEEM_SYMBOL) :
                                          dgp.current_sbd_supply *
                                          get_feed_history().current_median_history);

                    auto median_price = get_feed_history().current_median_history;

                    if (!median_price.is_null() &&
                        has_hardfork(STEEMIT_HARDFORK_0_14__230)) {
                        auto percent_sbd = uint16_t((
                                                            (fc::uint128_t((dgp.current_sbd_supply *
                                                                            get_feed_history().current_median_history).amount.value) *
                                                             STEEMIT_100_PERCENT)
                                                            / dgp.virtual_supply.amount.value).to_uint64());

                        if (percent_sbd <= STEEMIT_SBD_START_PERCENT) {
                            dgp.sbd_print_rate = STEEMIT_100_PERCENT;
                        } else if (percent_sbd >= STEEMIT_SBD_STOP_PERCENT) {
                            dgp.sbd_print_rate = 0;
                        } else {
                            dgp.sbd_print_rate =
                                    ((STEEMIT_SBD_STOP_PERCENT - percent_sbd) * STEEMIT_100_PERCENT) / (STEEMIT_SBD_STOP_PERCENT - STEEMIT_SBD_START_PERCENT);
                        }
                    }
                });
            } FC_CAPTURE_AND_RETHROW()
        }

        /**
     * Verifies all supply invariantes check out
     */
        void database_basic::validate_invariants() const {
            try {
                const auto &account_idx = get_index<account_index>().indices().get<by_name>();
                asset total_supply = asset(0, STEEM_SYMBOL);
                asset total_sbd = asset(0, SBD_SYMBOL);
                asset total_vesting = asset(0, VESTS_SYMBOL);
                share_type total_vsf_votes = share_type(0);

                auto gpo = get_dynamic_global_properties();

                /// verify no witness has too many votes
                const auto &witness_idx = get_index<witness_index>().indices();
                for (auto itr = witness_idx.begin(); itr != witness_idx.end(); ++itr)
                    FC_ASSERT(itr->votes < gpo.total_vesting_shares.amount, "", ("itr", *itr));

                for (auto itr = account_idx.begin();
                     itr != account_idx.end(); ++itr) {
                    total_supply += itr->balance;
                    total_supply += itr->savings_balance;
                    total_sbd += itr->sbd_balance;
                    total_sbd += itr->savings_sbd_balance;
                    total_vesting += itr->vesting_shares;
                    total_vsf_votes += (itr->proxy ==
                                        STEEMIT_PROXY_TO_SELF_ACCOUNT ?
                                        itr->witness_vote_weight() :
                                        (STEEMIT_MAX_PROXY_RECURSION_DEPTH > 0 ?
                                         itr->proxied_vsf_votes[
                                                 STEEMIT_MAX_PROXY_RECURSION_DEPTH -
                                                 1] :
                                         itr->vesting_shares.amount));
                }

                const auto &convert_request_idx = get_index<convert_request_index>().indices();

                for (auto itr = convert_request_idx.begin(); itr != convert_request_idx.end(); ++itr) {
                    if (itr->amount.symbol == STEEM_SYMBOL) {
                        total_supply += itr->amount;
                    } else if (itr->amount.symbol == SBD_SYMBOL) {
                        total_sbd += itr->amount;
                    } else
                        FC_ASSERT(false, "Encountered illegal symbol in convert_request_object");
                }

                const auto &limit_order_idx = get_index<limit_order_index>().indices();

                for (auto itr = limit_order_idx.begin(); itr != limit_order_idx.end(); ++itr) {
                    if (itr->sell_price.base.symbol == STEEM_SYMBOL) {
                        total_supply += asset(itr->for_sale, STEEM_SYMBOL);
                    } else if (itr->sell_price.base.symbol == SBD_SYMBOL) {
                        total_sbd += asset(itr->for_sale, SBD_SYMBOL);
                    }
                }

                const auto &escrow_idx = get_index<escrow_index>().indices().get<by_id>();

                for (auto itr = escrow_idx.begin(); itr != escrow_idx.end(); ++itr) {
                    total_supply += itr->steem_balance;
                    total_sbd += itr->sbd_balance;

                    if (itr->pending_fee.symbol == STEEM_SYMBOL) {
                        total_supply += itr->pending_fee;
                    } else if (itr->pending_fee.symbol == SBD_SYMBOL) {
                        total_sbd += itr->pending_fee;
                    } else
                        FC_ASSERT(false, "found escrow pending fee that is not SBD or STEEM");
                }

                const auto &savings_withdraw_idx = get_index<savings_withdraw_index>().indices().get<by_id>();

                for (auto itr = savings_withdraw_idx.begin();
                     itr != savings_withdraw_idx.end(); ++itr) {
                    if (itr->amount.symbol == STEEM_SYMBOL) {
                        total_supply += itr->amount;
                    } else if (itr->amount.symbol == SBD_SYMBOL) {
                        total_sbd += itr->amount;
                    } else
                        FC_ASSERT(false, "found savings withdraw that is not SBD or STEEM");
                }

                fc::uint128_t total_rshares2;
                fc::uint128_t total_children_rshares2;

                const auto &comment_idx = get_index<comment_index>().indices();

                for (auto itr = comment_idx.begin(); itr != comment_idx.end(); ++itr) {
                    if (itr->net_rshares.value > 0) {
                        auto delta = utilities::calculate_vshares(itr->net_rshares.value);
                        total_rshares2 += delta;
                    }
                    if (itr->parent_author == STEEMIT_ROOT_POST_PARENT) {
                        total_children_rshares2 += itr->children_rshares2;
                    }
                }

                const auto &reward_idx = get_index<reward_fund_index, by_id>();

                for (auto itr = reward_idx.begin();
                     itr != reward_idx.end(); ++itr) {
                    total_supply += itr->reward_balance;
                }

                total_supply += gpo.total_vesting_fund_steem + gpo.total_reward_fund_steem;

                FC_ASSERT(
                        gpo.current_supply == total_supply,
                        "",
                        ("gpo.current_supply", gpo.current_supply)("total_supply", total_supply)
                );
                FC_ASSERT(
                        gpo.current_sbd_supply == total_sbd,
                        "",
                        ("gpo.current_sbd_supply", gpo.current_sbd_supply)("total_sbd", total_sbd)
                );
                FC_ASSERT(
                        gpo.total_vesting_shares == total_vesting,
                        "",
                        ("gpo.total_vesting_shares", gpo.total_vesting_shares)("total_vesting", total_vesting)
                );
                FC_ASSERT(
                        gpo.total_vesting_shares.amount == total_vsf_votes,
                        "",
                        ("total_vesting_shares", gpo.total_vesting_shares)("total_vsf_votes", total_vsf_votes)
                );

                FC_ASSERT(gpo.virtual_supply >= gpo.current_supply);
                if (!get_feed_history().current_median_history.is_null()) {
                    FC_ASSERT(
                            gpo.current_sbd_supply * get_feed_history().current_median_history + gpo.current_supply == gpo.virtual_supply,
                            "",
                            ("gpo.current_sbd_supply", gpo.current_sbd_supply)("get_feed_history().current_median_history", get_feed_history().current_median_history)("gpo.current_supply", gpo.current_supply)("gpo.virtual_supply", gpo.virtual_supply)
                    );
                }
            }
            FC_CAPTURE_LOG_AND_RETHROW((head_block_num()));
        }


        uint32_t database_basic::head_block_num() const {
            return get_dynamic_global_properties().head_block_number;
        }

        block_id_type database_basic::head_block_id() const {
            return get_dynamic_global_properties().head_block_id;
        }

        uint32_t database_basic::last_non_undoable_block_num() const {
            return get_dynamic_global_properties().last_irreversible_block_num;
        }

        dynamic_extension::worker_storage &database_basic::dynamic_extension_worker() {
            return storage;
        }

        const dynamic_global_property_object &database_basic::get_dynamic_global_properties() const {
            try {
                return get<dynamic_global_property_object>();
            }
            FC_CAPTURE_AND_RETHROW()

        }

        const feed_history_object &database_basic::get_feed_history() const {
            try {
                return get<feed_history_object>();
            } FC_CAPTURE_AND_RETHROW()
        }

////Todo Nex Refactoring
}
} //steemit::chain
