#ifndef GOLOS_DYNAMIC_GLOBAL_PROPERTY_POLICY_HPP
#define GOLOS_DYNAMIC_GLOBAL_PROPERTY_POLICY_HPP
namespace steemit {
namespace chain {
struct dynamic_global_property_policy {

    dynamic_global_property_policy() = default;

    dynamic_global_property_policy(const behaviour_based_policy &) = default;

    dynamic_global_property_policy &operator=(const behaviour_based_policy &) = default;

    dynamic_global_property_policy(behaviour_based_policy &&) = default;

    dynamic_global_property_policy &operator=(behaviour_based_policy &&) = default;

    virtual ~dynamic_global_property_policy() = default;

    dynamic_global_property_policy(database_basic &ref, evaluator_registry <operation> &evaluator_registry_) : references(ref) {

    }

    const dynamic_global_property_object &get_dynamic_global_properties() const {
        try {
            return get<dynamic_global_property_object>();
        }
        FC_CAPTURE_AND_RETHROW()

    }

    time_point_sec head_block_time() const {
        return get_dynamic_global_properties().time;
    }

    void update_global_dynamic_data(const signed_block &b) {
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
                    const auto &witness_missed = get_witness(get_scheduled_witness(
                            i + 1));
                    if (witness_missed.owner != b.witness) {
                        modify(witness_missed, [&](witness_object &w) {
                            w.total_missed++;
                            if (has_hardfork(STEEMIT_HARDFORK_0_14__278)) {
                                if (head_block_num() -
                                    w.last_confirmed_block_num >
                                    STEEMIT_BLOCKS_PER_DAY) {
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
                    dgp.participation_count -= dgp.recent_slots_filled.hi &
                                               0x8000000000000000ULL ? 1
                                                                     : 0;
                    dgp.recent_slots_filled =
                            (dgp.recent_slots_filled << 1) +
                            (i == 0 ? 1 : 0);
                    dgp.participation_count += (i == 0 ? 1 : 0);
                }

                dgp.head_block_number = b.block_num();
                dgp.head_block_id = b.id();
                dgp.time = b.timestamp;
                dgp.current_aslot += missed_blocks + 1;
                dgp.average_block_size =
                        (99 * dgp.average_block_size + block_size) / 100;

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

            if (!(get_node_properties().skip_flags &
                  skip_undo_history_check)) {
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


protected:
    database_basic &references;

};
}}
#endif //GOLOS_DYNAMIC_GLOBAL_PROPERTY_POLICY_HPP
