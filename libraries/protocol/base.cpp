#include <steemit/protocol/base.hpp>

namespace steemit {
    namespace protocol {
        uint64_t base_operation::calculate_data_fee(uint64_t bytes, uint64_t price_per_kbyte) {
            auto result = (fc::uint128(bytes) * price_per_kbyte) / 1024;
            FC_ASSERT(result <= STEEMIT_MAX_SHARE_SUPPLY);
            return result.to_uint64();
        }
    }
}