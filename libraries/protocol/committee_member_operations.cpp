#include <steemit/protocol/committee_member_operations.hpp>

namespace steemit {
    namespace protocol {

        void committee_member_create_operation::validate() const {
            FC_ASSERT(url.size() < STEEMIT_MAX_URL_LENGTH);
        }

        void committee_member_update_operation::validate() const {
            if (new_url.valid())
                FC_ASSERT(new_url->size() < STEEMIT_MAX_URL_LENGTH);
        }

        void committee_member_update_global_parameters_operation::validate() const {
            new_parameters.validate();
        }

    }
}