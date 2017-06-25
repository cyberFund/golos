#ifndef COMMITTEE_MEMBER_OBJECT_HPP
#define COMMITTEE_MEMBER_OBJECT_HPP

#include <steemit/chain/steem_object_types.hpp>

#include <steemit/protocol/vote.hpp>

namespace steemit {
    namespace chain {
        /**
         *  @brief tracks information about a committee_member account.
         *  @ingroup object
         *
         *  A committee_member is responsible for setting blockchain parameters and has
         *  dynamic multi-sig control over the committee account.  The current set of
         *  active committee_members has control.
         *
         *  committee_members were separated into a separate object to make iterating over
         *  the set of committee_member easy.
         */
        class committee_member_object
                : public object<committee_member_object_type, committee_member_object> {
        public:
            committee_member_object() = delete;

            template<typename Constructor, typename Allocator>
            committee_member_object(Constructor &&c, allocator<Allocator> a) {
                c(*this);
            }

            id_type id;

            account_name_type committee_member_account;
            protocol::vote_id_type vote_id;
            uint64_t total_votes = 0;
            std::string url;
        };

        struct by_account;
        struct by_vote_id;
        typedef multi_index_container<
                committee_member_object,
                indexed_by<
                        ordered_unique<tag<by_id>,
                                member<committee_member_object, committee_member_object::id_type, &committee_member_object::id>
                        >,
                        ordered_unique<tag<by_account>,
                                member<committee_member_object, account_name_type, &committee_member_object::committee_member_account>
                        >,
                        ordered_unique<tag<by_vote_id>,
                                member<committee_member_object, protocol::vote_id_type, &committee_member_object::vote_id>
                        >
                >,
                allocator<committee_member_object>
        > committee_member_index;
    }
}

FC_REFLECT(steemit::chain::committee_member_object,
        (id)
                (committee_member_account)
                (vote_id)
                (total_votes)(url)
)
CHAINBASE_SET_INDEX_TYPE(steemit::chain::committee_member_object, steemit::chain::committee_member_index)

#endif //COMMITTEE_MEMBER_OBJECT_HPP