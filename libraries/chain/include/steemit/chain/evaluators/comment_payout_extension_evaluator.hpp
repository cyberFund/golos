#ifndef GOLOS_COMMENT_PAYOUT_EXTENSION_EVALUATOR_HPP
#define GOLOS_COMMENT_PAYOUT_EXTENSION_EVALUATOR_HPP

#include "forward.hpp"

namespace steemit {
    namespace chain {
        class comment_payout_extension_evaluator : public evaluator_impl<database_set,
                comment_payout_extension_evaluator> {
        public:
            typedef protocol::comment_payout_extension_operation operation_type;

            comment_payout_extension_evaluator(database_set &db) : evaluator_impl<database_set,
                    comment_payout_extension_evaluator>(db) {
            }

            void do_apply(const protocol::comment_payout_extension_operation &o) {
                const account_object &from_account = this->_db.get_account(o.payer);

                if (from_account.active_challenged) {
                    this->_db.modify(from_account, [&](account_object &a) {
                        a.active_challenged = false;
                        a.last_active_proved = this->_db.head_block_time();
                    });
                }

                const comment_object &comment = this->_db.get_comment(o.author, o.permlink);

                if (o.amount) {
                    FC_ASSERT(this->_db.get_balance(from_account, o.amount->symbol) >= *o.amount,
                              "Account does not have sufficient funds for transfer.");

                    this->_db.pay_fee(from_account, *o.amount);

                    this->_db.modify(comment, [&](comment_object &c) {
                        c.cashout_time = this->_db.get_payout_extension_time(comment, *o.amount);
                    });
                } else if (o.extension_time) {
                    asset amount = this->_db.get_payout_extension_cost(comment, *o.extension_time);
                    this->_db.pay_fee(from_account, amount);

                    this->_db.modify(comment, [&](comment_object &c) {
                        c.cashout_time = *o.extension_time;
                    });
                }
            }
        };
    }
}
#endif //GOLOS_COMMENT_PAYOUT_EXTENSION_EVALUATOR_HPP
