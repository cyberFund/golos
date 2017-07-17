#ifndef GOLOS_TRANSFORMER_HPP
#define GOLOS_TRANSFORMER_HPP

template<typename DataBase>
class transformer : public DataBase {
public:

    const feed_history_object &get_feed_history() const {
        try {
            return get<feed_history_object>();
        }FC_CAPTURE_AND_RETHROW()
    }

    const account_object &get_account(const account_name_type &name) const {
        try {
            return get<account_object, by_name>(name);
        } FC_CAPTURE_AND_RETHROW((name))
    }

    const account_object *find_account(const account_name_type &name) const {
        return find<account_object, by_name>(name);
    }

    const comment_object &get_comment(const account_name_type &author, const shared_string &permlink) const {
        try {
            return get<comment_object, by_permlink>(boost::make_tuple(author, permlink));
        } FC_CAPTURE_AND_RETHROW((author)(permlink))
    }

    const comment_object *find_comment(const account_name_type &author, const shared_string &permlink) const {
        return find<comment_object, by_permlink>(boost::make_tuple(author, permlink));
    }

    const comment_object &get_comment(const account_name_type &author, const string &permlink) const {
        try {
            return get<comment_object, by_permlink>(boost::make_tuple(author, permlink));
        } FC_CAPTURE_AND_RETHROW((author)(permlink))
    }

    const comment_object *find_comment(const account_name_type &author, const string &permlink) const {
        return find<comment_object, by_permlink>(boost::make_tuple(author, permlink));
    }

    const escrow_object &get_escrow(const account_name_type &name, uint32_t escrow_id) const {
        try {
            return get<escrow_object, by_from_id>(boost::make_tuple(name, escrow_id));
        } FC_CAPTURE_AND_RETHROW((name)(escrow_id))
    }

    const escrow_object *find_escrow(const account_name_type &name, uint32_t escrow_id) const {
        return find<escrow_object, by_from_id>(boost::make_tuple(name, escrow_id));
    }


    const limit_order_object &get_limit_order(const account_name_type &name, uint32_t orderid) const {
        try {
            if (!has_hardfork(STEEMIT_HARDFORK_0_6__127)) {
                orderid = orderid & 0x0000FFFF;
            }

            return get<limit_order_object, by_account>(boost::make_tuple(name, orderid));
        } FC_CAPTURE_AND_RETHROW((name)(orderid))
    }

    const limit_order_object *find_limit_order(const account_name_type &name, uint32_t orderid) const {
        if (!has_hardfork(STEEMIT_HARDFORK_0_6__127)) {
            orderid = orderid & 0x0000FFFF;
        }

        return find<limit_order_object, by_account>(boost::make_tuple(name, orderid));
    }


    const savings_withdraw_object &get_savings_withdraw(const account_name_type &owner, uint32_t request_id) const {
        try {
            return get<savings_withdraw_object, by_from_rid>(boost::make_tuple(owner, request_id));
        } FC_CAPTURE_AND_RETHROW((owner)(request_id))
    }

    const savings_withdraw_object *find_savings_withdraw(const account_name_type &owner, uint32_t request_id) const {
        return find<savings_withdraw_object, by_from_rid>(boost::make_tuple(owner, request_id));
    }

    const witness_schedule_object &get_witness_schedule_object() const {
        try {
            return get<witness_schedule_object>();
        } FC_CAPTURE_AND_RETHROW()
    }

    const witness_object &get_witness(const account_name_type &name) const {
        try {
            return get<witness_object, by_name>(name);
        } FC_CAPTURE_AND_RETHROW((name))
    }

    const witness_object *find_witness(const account_name_type &name) const {
        return find<witness_object, by_name>(name);
    }

    virtual ~transformer() = default;
};

#endif //GOLOS_TRANSFORMER_HPP
