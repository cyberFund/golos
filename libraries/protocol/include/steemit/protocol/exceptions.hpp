#pragma once

#include <fc/exception/exception.hpp>

#include <steemit/protocol/protocol.hpp>

#define STEEMIT_ASSERT(expr, exc_type, FORMAT, ...)                \
   FC_MULTILINE_MACRO_BEGIN                                           \
   if( !(expr) )                                                      \
      FC_THROW_EXCEPTION( exc_type, FORMAT, __VA_ARGS__ );            \
   FC_MULTILINE_MACRO_END

namespace steemit {
    namespace protocol {
        namespace exceptions {
            namespace transaction {
                template<uint32_t Code = 3000000, typename What = boost::mpl::string<'transaction exception'>>
                using base = fc::basic_exception<Code, What>;

                template<uint32_t Code = 3010000, typename What = boost::mpl::string<'missing required active authority'>>
                using tx_missing_active_auth = base<Code, What>;

                template<uint32_t Code = 3020000, typename What = boost::mpl::string<'missing required owner authority'>>
                using tx_missing_owner_auth = base<Code, What>;

                template<uint32_t Code = 3030000, typename What = boost::mpl::string<'missing required posting authority'>>
                using tx_missing_posting_auth = base<Code, What>;

                template<uint32_t Code = 3040000, typename What = boost::mpl::string<'missing required other authority'>>
                using tx_missing_other_auth = base<Code, What>;

                template<uint32_t Code = 3050000, typename What = boost::mpl::string<'irrelevant signature included'>>
                using tx_irrelevant_sig = base<Code, What>;

                template<uint32_t Code = 3060000, typename What = boost::mpl::string<'duplicate signature includedty'>>
                using tx_duplicate_sig = base<Code, What>;
            }
        }

#define STEEMIT_RECODE_EXC(cause_type, effect_type) \
      catch( const cause_type& e ) \
      { throw( effect_type( e.what(), e.get_log() ) ); }

    }
} // steemit::protocol