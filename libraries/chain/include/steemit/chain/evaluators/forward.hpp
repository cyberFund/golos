#include <steemit/protocol/steem_operations.hpp>
#include "evaluator.hpp"
#include <steemit/chain/custom_operation_interpreter.hpp>
#include <steemit/chain/steem_objects.hpp>
#include <steemit/chain/block_summary_object.hpp>
#include <steemit/chain/utilities/reward.hpp>
#include <steemit/chain/database/database_police.hpp>

#ifndef IS_LOW_MEM

#include <diff_match_patch.h>
#include <boost/locale/encoding_utf.hpp>
#include <steemit/chain/hardfork.hpp>

using boost::locale::conv::utf_to_utf;

std::wstring utf8_to_wstring(const std::string &str);

std::string wstring_to_utf8(const std::wstring &str);

#endif

namespace steemit {
    namespace chain {
        using namespace steemit::protocol;

        using fc::uint128_t;

        void validate_permlink_0_1(const string &permlink);

    }
}