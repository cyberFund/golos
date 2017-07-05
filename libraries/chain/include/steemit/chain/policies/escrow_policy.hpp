#ifndef GOLOS_ESCROW_OBJECT_POLICY_HPP
#define GOLOS_ESCROW_OBJECT_POLICY_HPP
namespace steemit {
namespace chain {
struct escrow_policy {

    escrow_policy() = default;

    escrow_policy(const escrow_policy &) = default;

    escrow_policy &operator=(const escrow_policy &) = default;

    escrow_policy(escrow_policy &&) = default;

    escrow_policy &operator=(escrow_policy &&) = default;

    virtual ~escrow_policy() = default;

    escrow_policy(database_basic &ref, evaluator_registry <operation> &evaluator_registry_) : references(ref) {

    }

    const escrow_object &get_escrow(const account_name_type &name, uint32_t escrow_id) const {
        try {
            return get<escrow_object, by_from_id>(boost::make_tuple(name, escrow_id));
        } FC_CAPTURE_AND_RETHROW((name)(escrow_id))
    }

    const escrow_object *find_escrow(const account_name_type &name, uint32_t escrow_id) const {
        return find<escrow_object, by_from_id>(boost::make_tuple(name, escrow_id));
    }

    void expire_escrow_ratification() {
        const auto &escrow_idx = get_index<escrow_index>().indices().get<by_ratification_deadline>();
        auto escrow_itr = escrow_idx.lower_bound(false);

        while (escrow_itr != escrow_idx.end() &&
               !escrow_itr->is_approved() &&
               escrow_itr->ratification_deadline <= head_block_time()) {
            const auto &old_escrow = *escrow_itr;
            ++escrow_itr;

            const auto &from_account = get_account(old_escrow.from);
            references.adjust_balance(from_account, old_escrow.steem_balance);
            references.adjust_balance(from_account, old_escrow.sbd_balance);
            references.adjust_balance(from_account, old_escrow.pending_fee);

            remove(old_escrow);
        }
    }

protected:
    database_basic &references;

};
}}
#endif //GOLOS_ESCROW_OBJECT_POLICY_HPP
