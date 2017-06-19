
#pragma once

#include <fc/api.hpp>
#include <fc/crypto/sha256.hpp>

#include <steemit/protocol/types.hpp>

#include <string>

namespace steemit {
    namespace application {
        struct api_context;
    }
}

namespace steemit {
    namespace plugin {
        namespace auth_util {

            struct check_authority_signature_params {
                std::string account_name;
                std::string level;
                fc::sha256 dig;
                std::vector<protocol::signature_type> sigs;
            };

            struct check_authority_signature_result {
                std::vector<protocol::public_key_type> keys;
            };

            class auth_util_api:public std::enable_shared_from_this<auth_util_api> {
            public:
                auth_util_api(const steemit::application::api_context &ctx);
                ~auth_util_api();
                void on_api_startup();

                check_authority_signature_result check_authority_signature(check_authority_signature_params args);

            private:
                struct auth_util_api_impl;
                std::unique_ptr<auth_util_api_impl> my;
            };

        }
    }
}

FC_REFLECT(steemit::plugin::auth_util::check_authority_signature_params,
        (account_name)
                (level)
                (dig)
                (sigs)
)
FC_REFLECT(steemit::plugin::auth_util::check_authority_signature_result,
        (keys)
)

FC_API(steemit::plugin::auth_util::auth_util_api,
        (check_authority_signature)
)
