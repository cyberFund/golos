#ifndef GOLOS_FEED_HISTORY_POLICY_HPP
#define GOLOS_FEED_HISTORY_POLICY_HPP

#include "generic_policy.hpp"

namespace steemit {
namespace chain {
struct feed_history_policy: public generic_policy {

    feed_history_policy() = default;

    feed_history_policy(const feed_history_policy &) = default;

    feed_history_policy &operator=(const feed_history_policy &) = default;

    feed_history_policy(feed_history_policy &&) = default;

    feed_history_policy &operator=(feed_history_policy &&) = default;

    virtual ~feed_history_policy() = default;

    feed_history_policy(database_basic &ref, evaluator_registry <operation> &evaluator_registry_) : generic_policy(ref) {
    }

    const feed_history_object &get_feed_history() const {
        try {
            return references.get<feed_history_object>();
        } FC_CAPTURE_AND_RETHROW()
    }


};
}}
#endif //GOLOS_FEED_HISTORY_POLICY_HPP
