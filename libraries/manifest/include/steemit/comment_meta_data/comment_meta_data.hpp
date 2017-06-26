#pragma once

#include <steemit/application/plugin.hpp>

#include <steemit/chain/database.hpp>
#include <steemit/chain/comment_object.hpp>

#include <boost/multi_index/composite_key.hpp>

#include <fc/thread/future.hpp>
#include <fc/api.hpp>

namespace steemit {

namespace application {
class discussion_query;

struct comment_api_obj;
}

namespace comment_meta_data {
using namespace steemit::chain;
using namespace boost::multi_index;

using steemit::application::application;

using chainbase::object;
using chainbase::oid;
using chainbase::allocator;

using name_type = fc::fixed_string<fc::sha256>;

/**
 *  The purpose of the tag object is to allow the generation and listing of
 *  all top level posts by a string tag.  The desired sort orders include:
 *
 *  1. created - time of creation
 *  2. maturing - about to receive a payout
 *  3. active - last reply the post or any child of the post
 *  4. netvotes - individual accounts voting for post minus accounts voting against it
 *
 *  When ever a comment is modified, all language_objects for that comment are updated to match.
 */

template<uint16_t TypeNumber>
class comment_meta_data_object : public object<TypeNumber, comment_meta_data_object<TypeNumber>> {
public:
    template<typename Constructor, typename Allocator>
    comment_meta_data_object(Constructor &&c, allocator<Allocator> a): name("") {
        c(*this);
        net_rshares = 0;
        net_votes = 0;
        children = 0;
        hot = 0;
        trending = 0;
        promoted_balance = 0;
    }

    comment_meta_data_object() : name("") {
        net_rshares = 0;
        net_votes = 0;
        children = 0;
        hot = 0;
        trending = 0;
        promoted_balance = 0;
    }

    id_type id;

    name_type name;
    time_point_sec created;
    time_point_sec active;
    time_point_sec cashout;
    int64_t net_rshares;
    int32_t net_votes;
    int32_t children;
    double hot;
    double trending;
    share_type promoted_balance;

    /**
     *  Used to track the total rshares^2 of all children, this is used for indexing purposes. A discussion
     *  that has a nested comment of high value should promote the entire discussion so that the comment can
     *  be reviewed.
     */
    fc::uint128_t children_rshares2;

    account_id_type author;
    comment_id_type parent;
    comment_id_type comment;

    bool is_post() const {
        return parent == comment_id_type();
    }
};

template<uint16_t TypeNumber>
using comment_meta_data_id_type = typename comment_meta_data_object<TypeNumber>::id_type;

template<typename T, typename C = std::less<T>>
class comparable_index {
public:
    typedef T value_type;

    virtual bool operator()(const T &first, const T &second) const = 0;
};

template<uint16_t TypeNumber>
class by_cashout : public comparable_index<comment_meta_data_object<TypeNumber>> {
public:
    virtual bool
    operator()(const comment_meta_data_object<TypeNumber> &first, const comment_meta_data_object<TypeNumber> &second) const override {
        return std::less<name_type>()(first.name, second.name) &&
               std::less<time_point_sec>()(first.cashout, second.cashout) &&
               std::less<typename comment_meta_data_object<TypeNumber>::id_type>()(first.id, second.id);
    }
}; /// all posts regardless of depth

template<uint16_t TypeNumber>
class by_net_rshares : public comparable_index<comment_meta_data_object<TypeNumber>> {
public:
    virtual bool
    operator()(const comment_meta_data_object<TypeNumber> &first, const comment_meta_data_object<TypeNumber> &second) const override {
        return std::greater<int64_t>()(first.net_rshares, second.net_rshares) &&
               std::less<typename comment_meta_data_object<TypeNumber>::id_type>()(first.id, second.id);
    }
}; /// all comments regardless of depth

template<uint16_t TypeNumber>
class by_parent_created : public comparable_index<comment_meta_data_object<TypeNumber>> {
public:
    virtual bool
    operator()(const comment_meta_data_object<TypeNumber> &first, const comment_meta_data_object<TypeNumber> &second) const override {
        return std::less<comment_id_type>()(first.parent, second.parent) &&
               std::greater<time_point_sec>()(first.created, second.created) &&
               std::less<typename comment_meta_data_object<TypeNumber>::id_type>()(first.id, second.id);
    }
};

template<uint16_t TypeNumber>
class by_parent_active : public comparable_index<comment_meta_data_object<TypeNumber>> {
public:
    virtual bool
    operator()(const comment_meta_data_object<TypeNumber> &first, const comment_meta_data_object<TypeNumber> &second) const override {
        return std::less<comment_id_type>()(first.parent, second.parent) &&
               std::greater<time_point_sec>()(first.active, second.active) &&
               std::less<typename comment_meta_data_object<TypeNumber>::id_type>()(first.id, second.id);
    }
};

template<uint16_t TypeNumber>
class by_parent_promoted : public comparable_index<comment_meta_data_object<TypeNumber>> {
public:
    virtual bool
    operator()(const comment_meta_data_object<TypeNumber> &first, const comment_meta_data_object<TypeNumber> &second) const override {
        return std::less<comment_id_type>()(first.parent, second.parent) &&
               std::greater<share_type>()(first.promoted_balance, second.promoted_balance) &&
               std::less<typename comment_meta_data_object<TypeNumber>::id_type>()(first.id, second.id);
    }
};

template<uint16_t TypeNumber>
class by_parent_net_rshares : public comparable_index<comment_meta_data_object<TypeNumber>> {
public:
    virtual bool
    operator()(const comment_meta_data_object<TypeNumber> &first, const comment_meta_data_object<TypeNumber> &second) const override {
        return std::less<comment_id_type>()(first.parent, second.parent) &&
               std::greater<int64_t>()(first.net_rshares, second.net_rshares) &&
               std::less<typename comment_meta_data_object<TypeNumber>::id_type>()(first.id, second.id);
    }
}; /// all top level posts by direct pending payout

template<uint16_t TypeNumber>
class by_parent_net_votes : public comparable_index<comment_meta_data_object<TypeNumber>> {
public:
    virtual bool
    operator()(const comment_meta_data_object<TypeNumber> &first, const comment_meta_data_object<TypeNumber> &second) const override {
        return std::less<comment_id_type>()(first.parent, second.parent) &&
               std::greater<int32_t>()(first.net_votes, second.net_votes) &&
               std::less<typename comment_meta_data_object<TypeNumber>::id_type>()(first.id, second.id);
    }
}; /// all top level posts by direct votes

template<uint16_t TypeNumber>
class by_parent_children_rshares2 : public comparable_index<comment_meta_data_object<TypeNumber>> {
public:
    virtual bool
    operator()(const comment_meta_data_object<TypeNumber> &first, const comment_meta_data_object<TypeNumber> &second) const override {
        return std::less<comment_id_type>()(first.parent, second.parent) &&
               std::greater<fc::uint128_t>()(first.children_rshares2, second.children_rshares2) &&
               std::less<typename comment_meta_data_object<TypeNumber>::id_type>()(first.id, second.id);
    }
}; /// all top level posts by total cumulative payout (aka payout)

template<uint16_t TypeNumber>
class by_parent_trending : public comparable_index<comment_meta_data_object<TypeNumber>> {
public:
    virtual bool
    operator()(const comment_meta_data_object<TypeNumber> &first, const comment_meta_data_object<TypeNumber> &second) const override {
        return std::less<comment_id_type>()(first.parent, second.parent) &&
               std::greater<double>()(first.trending, second.trending) &&
               std::less<typename comment_meta_data_object<TypeNumber>::id_type>()(first.id, second.id);
    }
};

template<uint16_t TypeNumber>
class by_parent_children : public comparable_index<comment_meta_data_object<TypeNumber>> {
public:
    virtual bool
    operator()(const comment_meta_data_object<TypeNumber> &first, const comment_meta_data_object<TypeNumber> &second) const override {
        return std::less<comment_id_type>()(first.parent, second.parent) &&
               std::greater<int32_t>()(first.children, second.children) &&
               std::less<typename comment_meta_data_object<TypeNumber>::id_type>()(first.id, second.id);
    }
}; /// all top level posts with the most discussion (replies at all levels)

template<uint16_t TypeNumber>
class by_parent_hot : public comparable_index<comment_meta_data_object<TypeNumber>> {
public:
    virtual bool
    operator()(const comment_meta_data_object<TypeNumber> &first, const comment_meta_data_object<TypeNumber> &second) const override {
        return std::less<comment_id_type>()(first.parent, second.parent) &&
               std::greater<double>()(first.hot, second.hot) &&
               std::less<typename comment_meta_data_object<TypeNumber>::id_type>()(first.id, second.id);
    }
};

template<uint16_t TypeNumber>
class by_author_parent_created : public comparable_index<comment_meta_data_object<TypeNumber>> {
public:
    virtual bool
    operator()(const comment_meta_data_object<TypeNumber> &first, const comment_meta_data_object<TypeNumber> &second) const override {
        return std::less<account_id_type>()(first.author, second.author) &&
               std::greater<time_point_sec>()(first.created, second.created) &&
               std::less<typename comment_meta_data_object<TypeNumber>::id_type>()(first.id, second.id);
    }
};  /// all blog posts by author with tag

template<uint16_t TypeNumber>
class by_author_comment : public comparable_index<comment_meta_data_object<TypeNumber>> {
public:
    virtual bool
    operator()(const comment_meta_data_object<TypeNumber> &first, const comment_meta_data_object<TypeNumber> &second) const override {
        return std::less<account_id_type>()(first.author, second.author) &&
               std::less<comment_id_type>()(first.comment, second.comment) &&
               std::less<typename comment_meta_data_object<TypeNumber>::id_type>()(first.id, second.id);
    }
};

template<uint16_t TypeNumber>
class by_reward_fund_net_rshares : public comparable_index<comment_meta_data_object<TypeNumber>> {
public:
    virtual bool
    operator()(const comment_meta_data_object<TypeNumber> &first, const comment_meta_data_object<TypeNumber> &second) const override {
        return std::less<bool>()(first.is_post(), second.is_post()) &&
               std::greater<int64_t>()(first.net_rshares, second.net_rshares) &&
               std::less<typename comment_meta_data_object<TypeNumber>::id_type>()(first.id, second.id);
    }
};

struct by_comment;
struct by_tag;

template<uint16_t TypeNumber>
using comment_meta_data_index = multi_index_container<
        comment_meta_data_object<TypeNumber>,
        indexed_by<
                ordered_unique<tag<by_id>, member<comment_meta_data_object<TypeNumber>, typename comment_meta_data_object<TypeNumber>::id_type, &comment_meta_data_object<TypeNumber>::id>>,
                ordered_non_unique<tag<by_comment>, member<comment_meta_data_object<TypeNumber>, comment_id_type, &comment_meta_data_object<TypeNumber>::comment>>,
                ordered_unique<tag<by_author_comment<TypeNumber>>,
                        composite_key<comment_meta_data_object<TypeNumber>,
                                member<comment_meta_data_object<TypeNumber>, account_id_type, &comment_meta_data_object<TypeNumber>::author>,
                                member<comment_meta_data_object<TypeNumber>, comment_id_type, &comment_meta_data_object<TypeNumber>::comment>,
                                member<comment_meta_data_object<TypeNumber>, typename comment_meta_data_object<TypeNumber>::id_type, &comment_meta_data_object<TypeNumber>::id>
                        >,
                        composite_key_compare<std::less<account_id_type>, std::less<comment_id_type>, std::less<typename comment_meta_data_object<TypeNumber>::id_type>>
                >,
                ordered_unique<tag<by_parent_created<TypeNumber>>,
                        composite_key<comment_meta_data_object<TypeNumber>,
                                member<comment_meta_data_object<TypeNumber>, name_type, &comment_meta_data_object<TypeNumber>::name>,
                                member<comment_meta_data_object<TypeNumber>, comment_id_type, &comment_meta_data_object<TypeNumber>::parent>,
                                member<comment_meta_data_object<TypeNumber>, time_point_sec, &comment_meta_data_object<TypeNumber>::created>,
                                member<comment_meta_data_object<TypeNumber>, typename comment_meta_data_object<TypeNumber>::id_type, &comment_meta_data_object<TypeNumber>::id>
                        >,
                        composite_key_compare<std::less<name_type>, std::less<comment_id_type>, std::greater<time_point_sec>, std::less<typename comment_meta_data_object<TypeNumber>::id_type>>
                >,
                ordered_unique<tag<by_parent_active<TypeNumber>>,
                        composite_key<comment_meta_data_object<TypeNumber>,
                                member<comment_meta_data_object<TypeNumber>, name_type, &comment_meta_data_object<TypeNumber>::name>,
                                member<comment_meta_data_object<TypeNumber>, comment_id_type, &comment_meta_data_object<TypeNumber>::parent>,
                                member<comment_meta_data_object<TypeNumber>, time_point_sec, &comment_meta_data_object<TypeNumber>::active>,
                                member<comment_meta_data_object<TypeNumber>, typename comment_meta_data_object<TypeNumber>::id_type, &comment_meta_data_object<TypeNumber>::id>
                        >,
                        composite_key_compare<std::less<name_type>, std::less<comment_id_type>, std::greater<time_point_sec>, std::less<typename comment_meta_data_object<TypeNumber>::id_type>>
                >,
                ordered_unique<tag<by_parent_promoted<TypeNumber>>,
                        composite_key<comment_meta_data_object<TypeNumber>,
                                member<comment_meta_data_object<TypeNumber>, name_type, &comment_meta_data_object<TypeNumber>::name>,
                                member<comment_meta_data_object<TypeNumber>, comment_id_type, &comment_meta_data_object<TypeNumber>::parent>,
                                member<comment_meta_data_object<TypeNumber>, share_type, &comment_meta_data_object<TypeNumber>::promoted_balance>,
                                member<comment_meta_data_object<TypeNumber>, typename comment_meta_data_object<TypeNumber>::id_type, &comment_meta_data_object<TypeNumber>::id>
                        >,
                        composite_key_compare<std::less<name_type>, std::less<comment_id_type>, std::greater<share_type>, std::less<typename comment_meta_data_object<TypeNumber>::id_type>>
                >,
                ordered_unique<tag<by_parent_net_rshares<TypeNumber>>,
                        composite_key<comment_meta_data_object<TypeNumber>,
                                member<comment_meta_data_object<TypeNumber>, name_type, &comment_meta_data_object<TypeNumber>::name>,
                                member<comment_meta_data_object<TypeNumber>, comment_id_type, &comment_meta_data_object<TypeNumber>::parent>,
                                member<comment_meta_data_object<TypeNumber>, int64_t, &comment_meta_data_object<TypeNumber>::net_rshares>,
                                member<comment_meta_data_object<TypeNumber>, typename comment_meta_data_object<TypeNumber>::id_type, &comment_meta_data_object<TypeNumber>::id>
                        >,
                        composite_key_compare<std::less<name_type>, std::less<comment_id_type>, std::greater<int64_t>, std::less<typename comment_meta_data_object<TypeNumber>::id_type>>
                >,
                ordered_unique<tag<by_parent_net_votes<TypeNumber>>,
                        composite_key<comment_meta_data_object<TypeNumber>,
                                member<comment_meta_data_object<TypeNumber>, name_type, &comment_meta_data_object<TypeNumber>::name>,
                                member<comment_meta_data_object<TypeNumber>, comment_id_type, &comment_meta_data_object<TypeNumber>::parent>,
                                member<comment_meta_data_object<TypeNumber>, int32_t, &comment_meta_data_object<TypeNumber>::net_votes>,
                                member<comment_meta_data_object<TypeNumber>, typename comment_meta_data_object<TypeNumber>::id_type, &comment_meta_data_object<TypeNumber>::id_type>
                        >,
                        composite_key_compare<std::less<name_type>, std::less<comment_id_type>, std::greater<int32_t>, std::less<typename comment_meta_data_object<TypeNumber>::id_type>>
                >,
                ordered_unique<tag<by_parent_children<TypeNumber>>,
                        composite_key<comment_meta_data_object<TypeNumber>,
                                member<comment_meta_data_object<TypeNumber>, name_type, &comment_meta_data_object<TypeNumber>::name>,
                                member<comment_meta_data_object<TypeNumber>, comment_id_type, &comment_meta_data_object<TypeNumber>::parent>,
                                member<comment_meta_data_object<TypeNumber>, int32_t, &comment_meta_data_object<TypeNumber>::children>,
                                member<comment_meta_data_object<TypeNumber>, typename comment_meta_data_object<TypeNumber>::id_type, &comment_meta_data_object<TypeNumber>::id>
                        >,
                        composite_key_compare<std::less<name_type>, std::less<comment_id_type>, std::greater<int32_t>, std::less<typename comment_meta_data_object<TypeNumber>::id_type>>
                >,
                ordered_unique<tag<by_parent_hot<TypeNumber>>,
                        composite_key<comment_meta_data_object<TypeNumber>,
                                member<comment_meta_data_object<TypeNumber>, name_type, &comment_meta_data_object<TypeNumber>::name>,
                                member<comment_meta_data_object<TypeNumber>, comment_id_type, &comment_meta_data_object<TypeNumber>::parent>,
                                member<comment_meta_data_object<TypeNumber>, double, &comment_meta_data_object<TypeNumber>::hot>,
                                member<comment_meta_data_object<TypeNumber>, typename comment_meta_data_object<TypeNumber>::id_type, &comment_meta_data_object<TypeNumber>::id>
                        >,
                        composite_key_compare<std::less<name_type>, std::less<comment_id_type>, std::greater<double>, std::less<typename comment_meta_data_object<TypeNumber>::id_type>>
                >,
                ordered_unique<tag<by_parent_trending<TypeNumber>>,
                        composite_key<comment_meta_data_object<TypeNumber>,
                                member<comment_meta_data_object<TypeNumber>, name_type, &comment_meta_data_object<TypeNumber>::name>,
                                member<comment_meta_data_object<TypeNumber>, comment_id_type, &comment_meta_data_object<TypeNumber>::parent>,
                                member<comment_meta_data_object<TypeNumber>, double, &comment_meta_data_object<TypeNumber>::trending>,
                                member<comment_meta_data_object<TypeNumber>, typename comment_meta_data_object<TypeNumber>::id_type, &comment_meta_data_object<TypeNumber>::id>
                        >,
                        composite_key_compare<std::less<name_type>, std::less<comment_id_type>, std::greater<double>, std::less<typename comment_meta_data_object<TypeNumber>::id_type>>
                >,
                ordered_unique<tag<by_parent_children_rshares2<TypeNumber>>,
                        composite_key<comment_meta_data_object<TypeNumber>,
                                member<comment_meta_data_object<TypeNumber>, name_type, &comment_meta_data_object<TypeNumber>::name>,
                                member<comment_meta_data_object<TypeNumber>, comment_id_type, &comment_meta_data_object<TypeNumber>::parent>,
                                member<comment_meta_data_object<TypeNumber>, fc::uint128_t, &comment_meta_data_object<TypeNumber>::children_rshares2>,
                                member<comment_meta_data_object<TypeNumber>, typename comment_meta_data_object<TypeNumber>::id_type, &comment_meta_data_object<TypeNumber>::id>
                        >,
                        composite_key_compare<std::less<name_type>, std::less<comment_id_type>, std::greater<fc::uint128_t>, std::less<typename comment_meta_data_object<TypeNumber>::id_type>>
                >,
                ordered_unique<tag<by_cashout<TypeNumber>>,
                        composite_key<comment_meta_data_object<TypeNumber>,
                                member<comment_meta_data_object<TypeNumber>, name_type, &comment_meta_data_object<TypeNumber>::name>,
                                member<comment_meta_data_object<TypeNumber>, time_point_sec, &comment_meta_data_object<TypeNumber>::cashout>,
                                member<comment_meta_data_object<TypeNumber>, typename comment_meta_data_object<TypeNumber>::id_type, &comment_meta_data_object<TypeNumber>::id>
                        >,
                        composite_key_compare<std::less<name_type>, std::less<time_point_sec>, std::less<typename comment_meta_data_object<TypeNumber>::id_type>>
                >,
                ordered_unique<tag<by_net_rshares<TypeNumber>>,
                        composite_key<comment_meta_data_object<TypeNumber>,
                                member<comment_meta_data_object<TypeNumber>, name_type, &comment_meta_data_object<TypeNumber>::name>,
                                member<comment_meta_data_object<TypeNumber>, int64_t, &comment_meta_data_object<TypeNumber>::net_rshares>,
                                member<comment_meta_data_object<TypeNumber>, typename comment_meta_data_object<TypeNumber>::id_type, &comment_meta_data_object<TypeNumber>::id>
                        >,
                        composite_key_compare<std::less<name_type>, std::greater<int64_t>, std::less<typename comment_meta_data_object<TypeNumber>::id_type>>
                >,
                ordered_unique<tag<by_author_parent_created<TypeNumber>>,
                        composite_key<comment_meta_data_object<TypeNumber>,
                                member<comment_meta_data_object<TypeNumber>, name_type, &comment_meta_data_object<TypeNumber>::name>,
                                member<comment_meta_data_object<TypeNumber>, account_id_type, &comment_meta_data_object<TypeNumber>::author>,
                                member<comment_meta_data_object<TypeNumber>, time_point_sec, &comment_meta_data_object<TypeNumber>::created>,
                                member<comment_meta_data_object<TypeNumber>, typename comment_meta_data_object<TypeNumber>::id_type, &comment_meta_data_object<TypeNumber>::id>
                        >,
                        composite_key_compare<std::less<name_type>, std::less<account_id_type>, std::greater<time_point_sec>, std::less<typename comment_meta_data_object<TypeNumber>::id_type>>
                >,
                ordered_unique<tag<by_reward_fund_net_rshares<TypeNumber>>,
                        composite_key<comment_meta_data_object<TypeNumber>,
                                member<comment_meta_data_object<TypeNumber>, name_type, &comment_meta_data_object<TypeNumber>::name>,
                                const_mem_fun<comment_meta_data_object<TypeNumber>, bool, &comment_meta_data_object<TypeNumber>::is_post>,
                                member<comment_meta_data_object<TypeNumber>, int64_t, &comment_meta_data_object<TypeNumber>::net_rshares>,
                                member<comment_meta_data_object<TypeNumber>, typename comment_meta_data_object<TypeNumber>::id_type, &comment_meta_data_object<TypeNumber>::id>
                        >,
                        composite_key_compare<std::less<name_type>, std::less<bool>, std::greater<int64_t>, std::less<typename comment_meta_data_object<TypeNumber>::id_type>>
                >
        >,
        allocator<comment_meta_data_object<TypeNumber>>
>;

/**
 *  The purpose of this index is to quickly identify how popular various tags by maintaining various sums over
 *  all posts under a particular tag
 */
template<uint16_t TypeNumber>
class comment_meta_data_stats_object : public object<TypeNumber, comment_meta_data_stats_object<TypeNumber>> {
public:
    template<typename Constructor, typename Allocator>
    comment_meta_data_stats_object(Constructor &&c, allocator<Allocator>):comment_meta_data_stats_object() {
        c(*this);
        total_payout = asset(0, SBD_SYMBOL);
        net_votes = 0;
        top_posts = 0;
        comments = 0;
    }

    comment_meta_data_stats_object() {
        total_payout = asset(0, SBD_SYMBOL);
        net_votes = 0;
        top_posts = 0;
        comments = 0;
    }

    id_type id;

    name_type name;
    fc::uint128_t total_children_rshares2;
    asset total_payout;
    int32_t net_votes;
    uint32_t top_posts;
    uint32_t comments;
};


struct by_comments;
struct by_top_posts;
struct by_trending;

template<uint16_t TypeNumber>
using comment_meta_data_stats_index = multi_index_container<
        comment_meta_data_stats_object<TypeNumber>,
        indexed_by<
                ordered_unique<tag<by_id>, member<comment_meta_data_stats_object<TypeNumber>,  typename comment_meta_data_stats_object<TypeNumber>::type_id, &comment_meta_data_stats_object<TypeNumber>::id>>,
                ordered_unique<tag<by_tag>, member<comment_meta_data_stats_object<TypeNumber>, name_type, &comment_meta_data_stats_object<TypeNumber>::name>>,
                ordered_non_unique<tag<by_trending>,
                        composite_key<comment_meta_data_stats_object<TypeNumber>,
                                member<comment_meta_data_stats_object<TypeNumber>, fc::uint128_t, &comment_meta_data_stats_object<TypeNumber>::total_children_rshares2>,
                                member<comment_meta_data_stats_object<TypeNumber>, name_type, &comment_meta_data_stats_object<TypeNumber>::name>
                        >,
                        composite_key_compare<std::greater<uint128_t>, std::less<name_type>>
                >
        >,
        allocator<comment_meta_data_stats_object<TypeNumber>>
> ;


/**
 *  The purpose of this object is to track the relationship between accounts based upon how a user votes. Every time
 *  a user votes on a post, the relationship between voter and author increases direct rshares.
 */
template<uint16_t TypeNumber>
class peer_stats_object : public object<TypeNumber, peer_stats_object<TypeNumber>> {
public:
    template<typename Constructor, typename Allocator>
    peer_stats_object(Constructor &&c, allocator<Allocator> a):peer_stats_object() {
        c(*this);
        direct_positive_votes = 0;
        direct_votes = 1;
        indirect_positive_votes = 0;
        indirect_votes = 1;
        rank = 0;
    }

    peer_stats_object() {
        direct_positive_votes = 0;
        direct_votes = 1;
        indirect_positive_votes = 0;
        indirect_votes = 1;
        rank = 0;
    }

    id_type id;

    account_id_type voter;
    account_id_type peer;
    int32_t direct_positive_votes;
    int32_t direct_votes;

    int32_t indirect_positive_votes;
    int32_t indirect_votes;

    float rank;

    void update_rank() {
        auto direct = float(direct_positive_votes) / direct_votes;
        auto indirect = float(indirect_positive_votes) / indirect_votes;
        auto direct_order = log(direct_votes);
        auto indirect_order = log(indirect_votes);

        if (!(direct_positive_votes + indirect_positive_votes)) {
            direct_order *= -1;
            indirect_order *= -1;
        }

        direct *= direct;
        indirect *= indirect;

        direct *= direct_order * 10;
        indirect *= indirect_order;

        rank = direct + indirect;
    }
};

struct by_rank;
struct by_voter_peer;

template<uint16_t TypeNumber>
using peer_stats_index =  multi_index_container<
        peer_stats_object<TypeNumber>,
        indexed_by<
                ordered_unique<tag<by_id>, member<peer_stats_object<TypeNumber>, typename peer_stats_object<TypeNumber>::id_type, &peer_stats_object<TypeNumber>::id>>,
                ordered_unique<tag<by_rank>,
                        composite_key<peer_stats_object<TypeNumber>,
                                member<peer_stats_object<TypeNumber>, account_id_type, &peer_stats_object<TypeNumber>::voter>,
                                member<peer_stats_object<TypeNumber>, float, &peer_stats_object<TypeNumber>::rank>,
                                member<peer_stats_object<TypeNumber>, account_id_type, &peer_stats_object<TypeNumber>::peer>
                        >,
                        composite_key_compare<std::less<account_id_type>, std::greater<float>, std::less<account_id_type>>
                >,
                ordered_unique<tag<by_voter_peer>,
                        composite_key<peer_stats_object<TypeNumber>,
                                member<peer_stats_object<TypeNumber>, account_id_type, &peer_stats_object<TypeNumber>::voter>,
                                member<peer_stats_object<TypeNumber>, account_id_type, &peer_stats_object<TypeNumber>::peer>
                        >,
                        composite_key_compare<std::less<account_id_type>, std::less<account_id_type>>
                >
        >,
        allocator<peer_stats_object<TypeNumber>>
>;

/**
 *  This purpose of this object is to maintain stats about which tags an author uses, how frequnetly, and
 *  how many total earnings of all posts by author in tag.  It also allows us to answer the question of which
 *  authors earn the most in each tag category.  This helps users to discover the best bloggers to follow for
 *  particular tags.
 */


template<uint16_t TypeNumber>
class author_comment_meta_data_stats_object : public object<TypeNumber, author_comment_meta_data_stats_object<TypeNumber>> {
public:
    template<typename Constructor, typename Allocator>
    author_comment_meta_data_stats_object(Constructor &&c, allocator<Allocator>)
            :author_comment_meta_data_stats_object() {
        c(*this);
        total_rewards = asset(0, SBD_SYMBOL);
        total_posts = 0;
    }

    author_comment_meta_data_stats_object() {
        total_rewards = asset(0, SBD_SYMBOL);
        total_posts = 0;
    }

    author_comment_meta_data_stats_object<TypeNumber>::id_type id;
    account_id_type author;
    name_type name;
    asset total_rewards;
    uint32_t total_posts;
};

struct by_author_tag_posts;
struct by_author_posts_tag;
struct by_author_tag_rewards;
struct by_tag_rewards_author;
using std::less;
using std::greater;

template<uint16_t TypeNumber>
using author_language_stats_index = chainbase::shared_multi_index_container<
        author_comment_meta_data_stats_object<TypeNumber>,
        indexed_by<
                ordered_unique<tag<by_id>,
                        member<author_comment_meta_data_stats_object<TypeNumber>, typename author_comment_meta_data_stats_object<TypeNumber>::id_type, &author_comment_meta_data_stats_object<TypeNumber>::id>
                >,
                ordered_unique<tag<by_author_posts_tag>,
                        composite_key<author_comment_meta_data_stats_object<TypeNumber>,
                                member<author_comment_meta_data_stats_object<TypeNumber>, account_id_type, &author_comment_meta_data_stats_object<TypeNumber>::author>,
                                member<author_comment_meta_data_stats_object<TypeNumber>, uint32_t, &author_comment_meta_data_stats_object<TypeNumber>::total_posts>,
                                member<author_comment_meta_data_stats_object<TypeNumber>, name_type, &author_comment_meta_data_stats_object<TypeNumber>::name>
                        >,
                        composite_key_compare<less<account_id_type>, greater<uint32_t>, less<name_type>>
                >,
                ordered_unique<tag<by_author_tag_posts>,
                        composite_key<author_comment_meta_data_stats_object<TypeNumber>,
                                member<author_comment_meta_data_stats_object<TypeNumber>, account_id_type, &author_comment_meta_data_stats_object<TypeNumber>::author>,
                                member<author_comment_meta_data_stats_object<TypeNumber>, name_type, &author_comment_meta_data_stats_object<TypeNumber>::name>,
                                member<author_comment_meta_data_stats_object<TypeNumber>, uint32_t, &author_comment_meta_data_stats_object<TypeNumber>::total_posts>
                        >,
                        composite_key_compare<less<account_id_type>, less<name_type>, greater<uint32_t>>
                >,
                ordered_unique<tag<by_author_tag_rewards>,
                        composite_key<author_comment_meta_data_stats_object<TypeNumber>,
                                member<author_comment_meta_data_stats_object<TypeNumber>, account_id_type, &author_comment_meta_data_stats_object<TypeNumber>::author>,
                                member<author_comment_meta_data_stats_object<TypeNumber>, name_type, &author_comment_meta_data_stats_object<TypeNumber>::name>,
                                member<author_comment_meta_data_stats_object<TypeNumber>, asset, &author_comment_meta_data_stats_object<TypeNumber>::total_rewards>
                        >,
                        composite_key_compare<less<account_id_type>, less<name_type>, greater<asset>>
                >,
                ordered_unique<tag<by_tag_rewards_author>,
                        composite_key<author_comment_meta_data_stats_object<TypeNumber>,
                                member<author_comment_meta_data_stats_object<TypeNumber>, name_type, &author_comment_meta_data_stats_object<TypeNumber>::name>,
                                member<author_comment_meta_data_stats_object<TypeNumber>, asset, &author_comment_meta_data_stats_object<TypeNumber>::total_rewards>,
                                member<author_comment_meta_data_stats_object<TypeNumber>, account_id_type, &author_comment_meta_data_stats_object<TypeNumber>::author>
                        >,
                        composite_key_compare<less<name_type>, greater<asset>, less<account_id_type>>
                >
        >
> ;

}
}