#include <steemit/protocol/steem_operations.hpp>

#include <steemit/chain/block_summary_object.hpp>
#include <steemit/chain/compound.hpp>
#include <steemit/chain/custom_operation_interpreter.hpp>
#include <steemit/chain/database.hpp>
#include <steemit/chain/database_exceptions.hpp>
#include <steemit/chain/db_with.hpp>
#include <steemit/chain/history_object.hpp>
#include <steemit/chain/steem_objects.hpp>
#include <steemit/chain/transaction_object.hpp>
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

        using boost::container::flat_set;

        database_basic::database_basic() {
        }

        database_basic::~database_basic() {
            clear_pending();
        }

        void database_basic::open(const fc::path &data_dir, const fc::path &shared_mem_dir, uint64_t initial_supply, uint64_t shared_file_size, uint32_t chainbase_flags) {
            try {
                init_schema();
                chainbase::database::open(shared_mem_dir, chainbase_flags, shared_file_size);

                initialize_indexes();
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

                    if (dynamic_global_property_.head_block_num()) {
                        auto head_block = _block_log.read_block_by_num(dynamic_global_property_.head_block_num());
                        // This assertion should be caught and a reindex should occur
                        FC_ASSERT(head_block.valid() && head_block->id() ==
                                                        head_block_id(), "Chain state does not match block log. Please reindex blockchain.");

                        _fork_db.start_block(*head_block);
                    }
                }

                with_read_lock([&]() {
                    hardfork_property_.init_hardforks(); // Writes to local state, but reads from db
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
                        skip_witness_signature |
                        skip_transaction_signatures |
                        skip_transaction_dupe_check |
                        skip_tapos_check |
                        skip_merkle_check |
                        skip_witness_schedule_check |
                        skip_authority_check |
                        skip_validate | /// no need to validate operations
                        skip_validate_invariants |
                        skip_block_log;

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

                if (!(skip & skip_fork_db)) {
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
            string scheduled_witness = get_scheduled_witness(slot_num);
            FC_ASSERT(scheduled_witness == witness_owner);

            const auto &witness_obj = get_witness(witness_owner);

            if (!(skip & skip_witness_signature))
                FC_ASSERT(witness_obj.signing_key ==
                          block_signing_private_key.get_public_key());

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
            if (hardfork_property_.has_hardfork(STEEMIT_HARDFORK_0_5__54)) {
                const auto &witness = get_witness(witness_owner);

                if (witness.running_version != STEEMIT_BLOCKCHAIN_VERSION) {
                    pending_block.extensions.insert(block_header_extensions(STEEMIT_BLOCKCHAIN_VERSION));
                }

                const auto &hfp = hardfork_property_.get_hardfork_property_object();

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

            if (!(skip & skip_witness_signature)) {
                pending_block.sign(block_signing_private_key);
            }

            // TODO:  Move this to _push_block() so session is restored.
            if (!(skip & skip_block_size_check)) {
                FC_ASSERT(fc::raw::pack_size(pending_block) <=
                          STEEMIT_MAX_BLOCK_SIZE);
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

        inline const void database_basic::push_virtual_operation(const operation &op, bool force) {
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

            int64_t head_block_abs_slot =
                    head_block_time().sec_since_epoch() / interval;
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
            /*done_adding_indexes();

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
                        db.node_properties().skip_flags |= skip_authority_check;
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
                        auth.account = STEEMIT_INIT_MINER_NAME +
                                       (i ? fc::to_string(i) : std::string());
                        auth.owner.add_authority(init_public_key, 1);
                        auth.owner.weight_threshold = 1;
                        auth.active = auth.owner;
                        auth.posting = auth.active;
                    });

                    create<witness_object>([&](witness_object &w) {
                        w.owner = STEEMIT_INIT_MINER_NAME +
                                  (i ? fc::to_string(i) : std::string());
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
                        skip = skip_witness_signature
                               | skip_transaction_signatures
                               | skip_transaction_dupe_check
                               | skip_fork_db
                               | skip_block_size_check
                               | skip_tapos_check
                               | skip_authority_check
                               /* | skip_merkle_check While blockchain is being downloaded, txs need to be validated against block headers */
                               | skip_undo_history_check
                               | skip_witness_schedule_check
                               | skip_validate
                               | skip_validate_invariants;
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

                if (!(skip & skip_merkle_check)) {
                    auto merkle_root = next_block.calculate_merkle_root();

                    try {
                        FC_ASSERT(next_block.transaction_merkle_root ==
                                  merkle_root, "Merkle check failed", ("next_block.transaction_merkle_root", next_block.transaction_merkle_root)("calc", merkle_root)("next_block", next_block)("id", next_block.id()));
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
                    FC_ASSERT(block_size <=
                              gprops.maximum_block_size, "Block Size is too Big", ("next_block_num", next_block_num)("block_size", block_size)("max", gprops.maximum_block_size));
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
                    const auto &witness = get_witness(next_block.witness);
                    const auto &hardfork_state = hardfork_property_.get_hardfork_property_object();
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
                        const auto &signing_witness = get_witness(next_block.witness);
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
                        const auto &signing_witness = get_witness(next_block.witness);
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

                if (!(skip &
                      skip_validate)) {   /* issue #505 explains why this skip_flag is disabled */
                    trx.validate();
                }

                auto &trx_idx = get_index<transaction_index>();
                const chain_id_type &chain_id = STEEMIT_CHAIN_ID;
                auto trx_id = trx.id();
                // idump((trx_id)(skip&skip_transaction_dupe_check));
                FC_ASSERT((skip & skip_transaction_dupe_check) ||
                          trx_idx.indices().get<by_trx_id>().find(trx_id) ==
                          trx_idx.indices().get<by_trx_id>().end(),
                        "Duplicate transaction check failed", ("trx_ix", trx_id));

                if (!(skip &
                      (skip_transaction_signatures | skip_authority_check))) {
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
                    const auto &acnt = get_account(auth);

                    if (!has_hardfork(STEEMIT_HARDFORK_0_17__79)) {
                        old_update_account_bandwidth(acnt, trx_size, bandwidth_type::old_forum);
                    }

                    update_account_bandwidth(acnt, trx_size, bandwidth_type::forum);
                    for (const auto &op : trx.operations) {
                        if (is_market_operation(op)) {
                            if (!has_hardfork(STEEMIT_HARDFORK_0_17__79)) {
                                old_update_account_bandwidth(acnt, trx_size, bandwidth_type::old_market);
                            }

                            update_account_bandwidth(acnt,
                                    trx_size * 10, bandwidth_type::market);
                            break;
                        }
                    }
                }

                //Skip all manner of expiration and TaPoS checking if we're on block 1; It's impossible that the transaction is
                //expired, and TaPoS makes no sense as no blocks exist.
                if (BOOST_LIKELY(head_block_num() > 0)) {
                    if (!(skip & skip_tapos_check)) {
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
                    if (is_producing() ||
                        has_hardfork(STEEMIT_HARDFORK_0_9)) // Simple solution to pending trx bug when now == trx.expiration
                        FC_ASSERT(now <
                                  trx.expiration, "", ("now", now)("trx.exp", trx.expiration));
                    FC_ASSERT(now <=
                              trx.expiration, "", ("now", now)("trx.exp", trx.expiration));
                }

                //Insert transaction into unique transactions database_basic.
                if (!(skip & skip_transaction_dupe_check)) {
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
                const witness_object &witness = get_witness(next_block.witness);

                if (!(skip & skip_witness_signature))
                    FC_ASSERT(next_block.validate_signee(witness.signing_key));

                if (!(skip & skip_witness_schedule_check)) {
                    uint32_t slot_num = get_slot_at_time(next_block.timestamp);
                    FC_ASSERT(slot_num > 0);

                    string scheduled_witness = get_scheduled_witness(slot_num);

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
                            _dpo.last_irreversible_block_num =
                                    head_block_num() - STEEMIT_MAX_WITNESSES;
                        }
                    });
                } else {
                    const witness_schedule_object &wso = get_witness_schedule_object();

                    vector<const witness_object *> wit_objs;
                    wit_objs.reserve(wso.num_scheduled_witnesses);
                    for (int i = 0; i < wso.num_scheduled_witnesses; i++) {
                        wit_objs.push_back(&get_witness(wso.current_shuffled_witnesses[i]));
                    }

                    static_assert(STEEMIT_IRREVERSIBLE_THRESHOLD >
                                  0, "irreversible threshold must be nonzero");

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

                if (!(get_node_properties().skip_flags & skip_block_log)) {
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

        database_basic::database_basic(hardfork_property_policy &hardfork_property,dynamic_global_property_policy&dynamic_global_property_) :
                hardfork_property_(hardfork_property),
                dynamic_global_property_(dynamic_global_property_)
                {}

}
} //steemit::chain
