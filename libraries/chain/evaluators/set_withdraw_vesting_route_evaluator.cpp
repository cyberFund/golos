#include <steemit/chain/evaluators/set_withdraw_vesting_route_evaluator.hpp>

void steemit::chain::set_withdraw_vesting_route_evaluator::do_apply(
        const protocol::set_withdraw_vesting_route_operation &o) {
    try {

        const auto &from_account = this->_db.get_account(o.from_account);
        const auto &to_account = this->_db.get_account(o.to_account);
        const auto &wd_idx = this->_db.get_index<withdraw_vesting_route_index>().indices().get<by_withdraw_route>();
        auto itr = wd_idx.find(boost::make_tuple(from_account.id, to_account.id));

        if (itr == wd_idx.end()) {
            FC_ASSERT(o.percent != 0, "Cannot create a 0% destination.");
            FC_ASSERT(from_account.withdraw_routes < STEEMIT_MAX_WITHDRAW_ROUTES,
                      "Account already has the maximum number of routes.");

            this->_db.create<withdraw_vesting_route_object>([&](withdraw_vesting_route_object &wvdo) {
                wvdo.from_account = from_account.id;
                wvdo.to_account = to_account.id;
                wvdo.percent = o.percent;
                wvdo.auto_vest = o.auto_vest;
            });

            this->_db.modify(from_account, [&](account_object &a) {
                a.withdraw_routes++;
            });
        } else if (o.percent == 0) {
            this->_db.remove(*itr);

            this->_db.modify(from_account, [&](account_object &a) {
                a.withdraw_routes--;
            });
        } else {
            this->_db.modify(*itr, [&](withdraw_vesting_route_object &wvdo) {
                wvdo.from_account = from_account.id;
                wvdo.to_account = to_account.id;
                wvdo.percent = o.percent;
                wvdo.auto_vest = o.auto_vest;
            });
        }

        itr = wd_idx.upper_bound(boost::make_tuple(from_account.id, account_id_type()));
        uint16_t total_percent = 0;

        while (itr->from_account == from_account.id && itr != wd_idx.end()) {
            total_percent += itr->percent;
            ++itr;
        }

        FC_ASSERT(total_percent <= STEEMIT_100_PERCENT,
                  "More than 100% of vesting withdrawals allocated to destinations.");
    } FC_CAPTURE_AND_RETHROW()
}
