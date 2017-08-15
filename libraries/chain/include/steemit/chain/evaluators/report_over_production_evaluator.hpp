#ifndef GOLOS_REPORT_OVER_PRODUCTION_EVALUATOR_HPP
#define GOLOS_REPORT_OVER_PRODUCTION_EVALUATOR_HPP

#include "forward.hpp"

namespace steemit {
    namespace chain {

        class report_over_production_evaluator : public evaluator_impl<database_tag, report_over_production_evaluator> {
        public:
            typedef protocol::report_over_production_operation operation_type;

            template<typename DataBase> report_over_production_evaluator(DataBase &db) : evaluator_impl<database_tag,
                    report_over_production_evaluator>(db) {
            }

            void do_apply(const protocol::report_over_production_operation &o);
        };
    }
}
#endif //GOLOS_REPORT_OVER_PRODUCTION_EVALUATOR_HPP
