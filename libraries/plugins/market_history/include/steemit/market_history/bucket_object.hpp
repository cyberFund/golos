#ifndef GOLOS_BUCKET_OBJECT_HPP
#define GOLOS_BUCKET_OBJECT_HPP

#include <steemit/application/plugin.hpp>

#include <steemit/chain/steem_object_types.hpp>

#include <steemit/market_history/market_history_object_types.hpp>
#include <steemit/market_history/key_interface.hpp>

#include <steemit/protocol/asset.hpp>

namespace steemit {
    namespace market_history {
        class bucket_key : public key_interface {
        public:
            bucket_key(protocol::asset_name_type a, protocol::asset_name_type b, uint32_t s, fc::time_point_sec o)
                    : key_interface(a, b), seconds(s), open(o) {
            }

            bucket_key();

            uint32_t seconds = 0;
            fc::time_point_sec open;

            friend bool operator<(const bucket_key &a, const bucket_key &b) {
                return std::tie(a.base, a.quote, a.seconds, a.open) <
                       std::tie(b.base, b.quote, b.seconds, b.open);
            }

            friend bool operator==(const bucket_key &a, const bucket_key &b) {
                return std::tie(a.base, a.quote, b.seconds, a.open) ==
                       std::tie(b.base, b.quote, b.seconds, b.open);
            }
        };

        struct by_id;
        struct by_bucket;
        struct by_key;

        struct bucket_object
                : public chain::object<bucket_object_type, bucket_object> {
            template<typename Constructor, typename Allocator>
            bucket_object(Constructor &&c, chainbase::allocator<Allocator> a) {
                c(*this);
            }

            id_type id;

            fc::time_point_sec open;
            uint32_t seconds = 0;
            protocol::share_type high_steem;
            protocol::share_type high_sbd;
            protocol::share_type low_steem;
            protocol::share_type low_sbd;
            protocol::share_type open_steem;
            protocol::share_type open_sbd;
            protocol::share_type close_steem;
            protocol::share_type close_sbd;
            protocol::share_type steem_volume;
            protocol::share_type sbd_volume;

            protocol::price high() const {
                return protocol::asset(high_sbd, SBD_SYMBOL) /
                       protocol::asset(high_steem, STEEM_SYMBOL);
            }

            protocol::price low() const {
                return protocol::asset(low_sbd, SBD_SYMBOL) /
                       protocol::asset(low_steem, STEEM_SYMBOL);
            }
        };

        typedef multi_index_container<
                bucket_object,
                indexed_by<
                        ordered_unique<tag<
                                by_id>, member<bucket_object, bucket_object::id_type,
                                &bucket_object::id>>,
                        ordered_unique<tag<by_bucket>,
                                composite_key<bucket_object,
                                        member<
                                                bucket_object, uint32_t,
                                                &bucket_object::seconds>,
                                        member<bucket_object, fc::time_point_sec,
                                                &bucket_object::open>
                                >,
                                composite_key_compare<std::less<uint32_t>, std::less<fc::time_point_sec>>
                        >
                >,
                chainbase::allocator<bucket_object>
        > bucket_index;

        struct asset_bucket_object
                : public chain::object<asset_bucket_object_type, asset_bucket_object> {
            template<typename Constructor, typename Allocator>
            asset_bucket_object(Constructor &&c, chainbase::allocator<Allocator> a) {
                c(*this);
            }

            id_type id;

            protocol::price high() const {
                return protocol::asset(high_base, key.base) /
                       protocol::asset(high_quote, key.quote);
            }

            protocol::price low() const {
                return protocol::asset(low_base, key.base) /
                       protocol::asset(low_quote, key.quote);
            }

            bucket_key key;
            protocol::share_type high_base;
            protocol::share_type high_quote;
            protocol::share_type low_base;
            protocol::share_type low_quote;
            protocol::share_type open_base;
            protocol::share_type open_quote;
            protocol::share_type close_base;
            protocol::share_type close_quote;
            protocol::share_type base_volume;
            protocol::share_type quote_volume;
        };

        typedef multi_index_container<
                asset_bucket_object,
                indexed_by<
                        ordered_unique<tag<by_id>, member<asset_bucket_object, asset_bucket_object::id_type, &asset_bucket_object::id>>,
                        ordered_unique<tag<by_key>, member<asset_bucket_object, bucket_key, &asset_bucket_object::key>>
                >, chainbase::allocator<asset_bucket_object>
        > asset_bucket_index;
    }
}

FC_REFLECT_DERIVED(steemit::market_history::bucket_key, (steemit::market_history::key_interface), (seconds)(open));

FC_REFLECT(steemit::market_history::bucket_object,
        (id)
                (open)(seconds)
                (high_steem)(high_sbd)
                (low_steem)(low_sbd)
                (open_steem)(open_sbd)
                (close_steem)(close_sbd)
                (steem_volume)(sbd_volume));
CHAINBASE_SET_INDEX_TYPE(steemit::market_history::bucket_object, steemit::market_history::bucket_index);

FC_REFLECT(steemit::market_history::asset_bucket_object,
        (id)(key)
                (high_base)(high_quote)
                (low_base)(low_quote)
                (open_base)(open_quote)
                (close_base)(close_quote)
                (base_volume)(quote_volume))
CHAINBASE_SET_INDEX_TYPE(
        steemit::market_history::asset_bucket_object, steemit::market_history::asset_bucket_index);

#endif //GOLOS_BUCKET_OBJECT_HPP