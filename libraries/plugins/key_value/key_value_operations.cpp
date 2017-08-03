#include <steemit/key_value/key_value_operations.hpp>

namespace steemit {
    namespace key_value {
        inline void validate_account_name(const string &name) {
            FC_ASSERT(is_valid_account_name(name), "Account name ${n} is invalid", ("n", name));
        }

        void create_first_key_value_operation::validate() const {
            validate_account_name(owner);
        }

        void update_first_key_value_operation::validate() const {
            validate_account_name(owner);
        }

        void delete_first_key_value_operation::validate() const {
            validate_account_name(owner);
        }
    }
}
