#ifndef GOLOS_COMMENT_OPTIONS_EVALUATOR_HPP
#define GOLOS_COMMENT_OPTIONS_EVALUATOR_HPP

#include "forward.hpp"

namespace steemit {
    namespace chain {

        class comment_options_evaluator : public evaluator_impl<database_tag, comment_options_evaluator> {
        private:
            struct comment_options_extension_visitor {
                comment_options_extension_visitor(const comment_object &c, database_tag &db) : _c(c), _db(db) {
                }

                typedef void result_type;

                const comment_object &_c;
                database_tag &_db;

                void operator()(const comment_payout_beneficiaries &cpb) const {
                    if (this->_db.is_producing()) {
                        FC_ASSERT(cpb.beneficiaries.size() <= 8, "Cannot specify more than 8 beneficiaries.");
                    }

                    FC_ASSERT(_c.beneficiaries.size() == 0, "Comment already has beneficiaries specified.");
                    FC_ASSERT(_c.abs_rshares == 0,
                              "Comment must not have been voted on before specifying beneficiaries.");

                    this->_db.modify(_c, [&](comment_object &c) {
                        for (auto &b : cpb.beneficiaries) {
                            auto acc = this->_db.find<account_object, by_name>(b.account);
                            FC_ASSERT(acc != nullptr, "Beneficiary \"${a}\" must exist.", ("a", b.account));
                            c.beneficiaries.push_back(b);
                        }
                    });
                }
            };

        public:
            typedef protocol::comment_options_operation operation_type;

            template<typename DataBase>
            comment_options_evaluator(DataBase &db) : evaluator_impl<database_tag, comment_options_evaluator>(db) {
            }

            void do_apply(const protocol::comment_options_operation &o);
        };
    }
}
#endif //GOLOS_COMMENT_OPTIONS_EVALUATOR_HPP
