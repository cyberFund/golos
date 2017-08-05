#include <steemit/chain/evaluators/custom_json_evaluator.hpp>
void steemit::chain::custom_json_evaluator::do_apply(const protocol::custom_json_operation &o) {
    auto &d = db();
    std::shared_ptr <custom_operation_interpreter> eval = d.get_custom_json_evaluator(o.id);
    if (!eval) {
        return;
    }

    try {
        eval->apply(o);
    }
    catch (const fc::exception &e) {
        if (d.is_producing()) {
            throw e;
        }
    }
    catch (...) {
        elog("Unexpected exception applying custom json evaluator.");
    }
}
