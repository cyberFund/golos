#include <steemit/chain/evaluators/custom_binary_evaluator.hpp>

void steemit::chain::custom_binary_evaluator::do_apply(const protocol::custom_binary_operation &o) {
    auto &d = db();
    FC_ASSERT(d.has_hardfork(STEEMIT_HARDFORK_0_14__317));

    std::shared_ptr<custom_operation_interpreter> eval = d.get_custom_json_evaluator(o.id);
    if (!eval) {
        return;
    }

    try {
        eval->apply(o);
    } catch (const fc::exception &e) {
        if (d.is_producing()) {
            throw e;
        }
    } catch (...) {
        elog("Unexpected exception applying custom json evaluator.");
    }
}
