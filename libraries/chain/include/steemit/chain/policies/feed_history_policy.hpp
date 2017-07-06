#ifndef GOLOS_FEED_HISTORY_POLICY_HPP
#define GOLOS_FEED_HISTORY_POLICY_HPP
namespace steemit {
namespace chain {
struct feed_history_policy {

    feed_history_policy() = default;

    feed_history_policy(const feed_history_policy &) = default;

    feed_history_policy &operator=(const feed_history_policy &) = default;

    feed_history_policy(feed_history_policy &&) = default;

    feed_history_policy &operator=(feed_history_policy &&) = default;

    virtual ~feed_history_policy() = default;

    feed_history_policy(database_basic &ref, evaluator_registry <operation> &evaluator_registry_) : references(ref) {
    }

    const feed_history_object &get_feed_history() const {
        try {
            return get<feed_history_object>();
        } FC_CAPTURE_AND_RETHROW()
    }

protected:
    database_basic &references;

};
}}
#endif //GOLOS_FEED_HISTORY_POLICY_HPP
