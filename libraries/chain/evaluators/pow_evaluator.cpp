#include <steemit/chain/evaluators/pow_evaluator.hpp>
void steemit::chain::pow_evaluator::do_apply(const protocol::pow_operation &o) {
    FC_ASSERT(!db().has_hardfork(STEEMIT_HARDFORK_0_13__256), "pow is deprecated. Use pow2 instead");
    pow_apply(db(), o);
}
