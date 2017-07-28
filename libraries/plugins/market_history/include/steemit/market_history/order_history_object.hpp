#ifndef GOLOS_ORDER_HISTORY_OBJECT_HPP
#define GOLOS_ORDER_HISTORY_OBJECT_HPP

#include <steemit/application/plugin.hpp>

#include <steemit/chain/steem_object_types.hpp>

#include <steemit/market_history/market_history_object_types.hpp>
#include <steemit/market_history/key_interface.hpp>

#include <steemit/protocol/asset.hpp>

namespace steemit {
    namespace market_history {
        class history_key : public key_interface {
        public:
            history_key();

            history_key(protocol::asset_name_type base, protocol::asset_name_type quote, int64_t sequence)
                    : key_interface(base, quote), sequence(sequence) {
            }

            int64_t sequence = 0;

            friend bool operator<(const history_key &a, const history_key &b) {
                return std::tie(a.base, a.quote, a.sequence) <
                       std::tie(b.base, b.quote, b.sequence);
            }

            friend bool operator==(const history_key &a, const history_key &b) {
                return std::tie(a.base, a.quote, a.sequence) ==
                       std::tie(b.base, b.quote, b.sequence);
            }
        };

        struct order_history_object
                : public chain::object<order_history_object_type, order_history_object> {
            template<typename Constructor, typename Allocator>
            order_history_object(Constructor &&c, chainbase::allocator<Allocator> a) {
                c(*this);
            }

            id_type id;

            fc::time_point_sec time;
            protocol::fill_order_operation op;
        };

        struct asset_order_history_object
                : public chain::object<asset_order_history_object_type, asset_order_history_object> {
            template<typename Constructor, typename Allocator>
            asset_order_history_object(Constructor &&c, chainbase::allocator<Allocator> a) {
                c(*this);
            }

            id_type id;

            history_key key;
            fc::time_point_sec time;
            chain::fill_asset_order_operation op;
        };

        struct by_time;
        struct by_key;

        typedef multi_index_container<
                order_history_object,
                indexed_by<
                        ordered_unique<tag<chain::by_id>, member<order_history_object, order_history_object::id_type, &order_history_object::id>>,
                        ordered_non_unique<tag<by_time>, member<order_history_object, time_point_sec, &order_history_object::time>>
                >,
                chainbase::allocator<order_history_object>
        > order_history_index;

        typedef multi_index_container<
                asset_order_history_object,
                indexed_by<
                        ordered_unique<tag<chain::by_id>, member<asset_order_history_object, asset_order_history_object::id_type, &asset_order_history_object::id>>,
                        ordered_unique<tag<by_key>, member<asset_order_history_object, history_key, &asset_order_history_object::key>>
                >, chainbase::allocator<asset_order_history_object>
        > asset_order_history_index;

    }
}

FC_REFLECT(steemit::market_history::order_history_object,
        (id)
                (time)
                (op));
CHAINBASE_SET_INDEX_TYPE(steemit::market_history::order_history_object, steemit::market_history::order_history_index);

FC_REFLECT_DERIVED(steemit::market_history::history_key, (steemit::market_history::key_interface), (sequence));

FC_REFLECT(steemit::market_history::asset_order_history_object, (key)(time)(op));
CHAINBASE_SET_INDEX_TYPE(steemit::market_history::asset_order_history_object, steemit::market_history::asset_order_history_index);

#endif //GOLOS_ORDER_HISTORY_OBJECT_HPP
