#include <steemit/chain/evaluators/report_over_production_evaluator.hpp>

void steemit::chain::report_over_production_evaluator::do_apply(const protocol::report_over_production_operation &o) {

    FC_ASSERT(!this->_db.has_hardfork(STEEMIT_HARDFORK_0_4), "report_over_production_operation is disabled.");
}
