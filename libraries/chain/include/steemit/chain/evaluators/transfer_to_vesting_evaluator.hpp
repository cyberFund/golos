#ifndef GOLOS_TRANSFER_TO_VESTING_EVALUATOR_HPP
#define GOLOS_TRANSFER_TO_VESTING_EVALUATOR_HPP

#include "forward.hpp"

namespace steemit {
    namespace chain {

        class transfer_to_vesting_evaluator : public evaluator_impl<database_tag, transfer_to_vesting_evaluator> {
        public:
            typedef protocol::transfer_to_vesting_operation operation_type;

            template<typename Database>
            transfer_to_vesting_evaluator(Database &db) : evaluator_impl<database_tag, transfer_to_vesting_evaluator>(
                    db) {
            }

            void do_apply(const transfer_to_vesting_operation &o);

        };
    }
}
#endif //GOLOS_TRANSFER_TO_VESTING_EVALUATOR_HPP
