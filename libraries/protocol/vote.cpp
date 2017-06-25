#include <steemit/protocol/vote.hpp>

namespace fc {
    void to_variant(const steemit::protocol::vote_id_type &var, variant &vo) {
        vo = std::string(var);
    }

    void from_variant(const variant &var, steemit::protocol::vote_id_type &vo) {
        vo = steemit::protocol::vote_id_type(var.as_string());
    }
}