#include <steemit/chain/database/generic_policy.hpp>

namespace steemit {
namespace chain {

generic_policy::generic_policy(database_basic &references)
        : references(references) {}

}}