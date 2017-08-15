#include <steemit/chain/evaluators/forward.hpp>


#include <steemit/protocol/steem_operations.hpp>
#include <steemit/chain/custom_operation_interpreter.hpp>
#include <steemit/chain/chain_objects/steem_objects.hpp>
#include <steemit/chain/chain_objects/block_summary_object.hpp>
#include <steemit/chain/utilities/reward.hpp>

#ifndef IS_LOW_MEM

#include <diff_match_patch.h>
#include <boost/locale/encoding_utf.hpp>
#include <steemit/chain/hardfork.hpp>

using boost::locale::conv::utf_to_utf;

std::wstring utf8_to_wstring(const std::string &str) {
    return utf_to_utf<wchar_t>(str.c_str(), str.c_str() + str.size());
}

std::string wstring_to_utf8(const std::wstring &str) {
    return utf_to_utf<char>(str.c_str(), str.c_str() + str.size());
}

#endif

namespace steemit {
    namespace chain {
        void validate_permlink_0_1(const string &permlink) {
            FC_ASSERT(permlink.size() > STEEMIT_MIN_PERMLINK_LENGTH && permlink.size() < STEEMIT_MAX_PERMLINK_LENGTH,
                      "Permlink is not a valid size.");

            for (auto c : permlink) {
                switch (c) {
                    case 'a':
                    case 'b':
                    case 'c':
                    case 'd':
                    case 'e':
                    case 'f':
                    case 'g':
                    case 'h':
                    case 'i':
                    case 'j':
                    case 'k':
                    case 'l':
                    case 'm':
                    case 'n':
                    case 'o':
                    case 'p':
                    case 'q':
                    case 'r':
                    case 's':
                    case 't':
                    case 'u':
                    case 'v':
                    case 'w':
                    case 'x':
                    case 'y':
                    case 'z':
                    case '0':
                    case '1':
                    case '2':
                    case '3':
                    case '4':
                    case '5':
                    case '6':
                    case '7':
                    case '8':
                    case '9':
                    case '-':
                        break;
                    default:
                        FC_ASSERT(false, "Invalid permlink character: ${s}", ("s", std::string() + c));
                }
            }
        }
    }
}