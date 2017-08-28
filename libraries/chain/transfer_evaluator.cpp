#include <steemit/chain/transfer_evaluator.hpp>

#include <steemit/chain/account_object.hpp>
#include <steemit/chain/asset_object.hpp>

#include <steemit/chain/database.hpp>
#include <steemit/chain/database_exceptions.hpp>

namespace steemit {
    namespace chain {
        void transfer_evaluator::do_apply(const protocol::transfer_operation &o) {

            const auto &from_account = db.get_account(o.from);
            const auto &to_account = db.get_account(o.to);

            const asset_object &asset_type = db.get_asset(o.amount.symbol_name());

            STEEMIT_ASSERT(
                    db.is_authorized_asset(from_account, asset_type),
                    transfer_from_account_not_whitelisted,
                    "'from' account ${from} is not whitelisted for asset ${asset}",
                    ("from", o.from)
                            ("asset", o.amount.symbol)
            );
            STEEMIT_ASSERT(
                    db.is_authorized_asset(to_account, asset_type),
                    transfer_to_account_not_whitelisted,
                    "'to' account ${to} is not whitelisted for asset ${asset}",
                    ("to", o.to)
                            ("asset", o.amount.symbol)
            );

            if (asset_type.is_transfer_restricted()) {
                STEEMIT_ASSERT(
                        from_account.name == asset_type.issuer ||
                        to_account.name == asset_type.issuer,
                        transfer_restricted_transfer_asset,
                        "Asset {asset} has transfer_restricted flag enabled",
                        ("asset", o.amount.symbol)
                );
            }

            if (from_account.active_challenged) {
                this->db.modify(from_account, [&](account_object &a) {
                    a.active_challenged = false;
                    a.last_active_proved = this->db.head_block_time();
                });
            }

            FC_ASSERT(this->db.get_balance(from_account, o.amount.symbol_name()) >=
                      o.amount, "Account does not have sufficient funds for transfer.");

            this->db.adjust_balance(from_account, -o.amount);
            this->db.adjust_balance(to_account, o.amount);
        }

        void transfer_to_vesting_evaluator::do_apply(const protocol::transfer_to_vesting_operation &o) {
            const auto &from_account = this->db.get_account(o.from);
            const auto &to_account = o.to.size() ? this->db.get_account(o.to)
                                                 : from_account;

            FC_ASSERT(this->db.get_balance(from_account, STEEM_SYMBOL_NAME) >=
                      o.amount, "Account does not have sufficient {a} for transfer.", ("a", STEEM_SYMBOL_NAME));
            this->db.adjust_balance(from_account, -o.amount);
            this->db.create_vesting(to_account, o.amount);
        }

        void transfer_to_savings_evaluator::do_apply(const transfer_to_savings_operation &op) {
            const auto &from = this->db.get_account(op.from);
            const auto &to = this->db.get_account(op.to);
            FC_ASSERT(this->db.get_balance(from, op.amount.symbol_name()) >=
                      op.amount, "Account does not have sufficient funds to transfer to savings.");

            this->db.adjust_balance(from, -op.amount);
            this->db.adjust_savings_balance(to, op.amount);
        }

        void transfer_from_savings_evaluator::do_apply(const protocol::transfer_from_savings_operation &op) {
            const auto &from = this->db.get_account(op.from);
            this->db.get_account(op.to); // Verify to account exists

            FC_ASSERT(from.savings_withdraw_requests <
                      STEEMIT_SAVINGS_WITHDRAW_REQUEST_LIMIT, "Account has reached limit for pending withdraw requests.");

            FC_ASSERT(this->db.get_savings_balance(from, op.amount.symbol_name()) >=
                      op.amount);
            this->db.adjust_savings_balance(from, -op.amount);
            this->db.create<savings_withdraw_object>([&](savings_withdraw_object &s) {
                s.from = op.from;
                s.to = op.to;
                s.amount = op.amount;
#ifndef STEEMIT_BUILD_LOW_MEMORY
                from_string(s.memo, op.memo);
#endif
                s.request_id = op.request_id;
                s.complete = this->db.head_block_time() +
                        STEEMIT_SAVINGS_WITHDRAW_TIME;
            });

            this->db.modify(from, [&](account_object &a) {
                a.savings_withdraw_requests++;
            });
        }

        void cancel_transfer_from_savings_evaluator::do_apply(const protocol::cancel_transfer_from_savings_operation &op) {
            const auto &swo = this->db.get_savings_withdraw(op.from, op.request_id);
            this->db.adjust_savings_balance(this->db.get_account(swo.from), swo.amount);
            this->db.remove(swo);

            const auto &from = this->db.get_account(op.from);
            this->db.modify(from, [&](account_object &a) {
                a.savings_withdraw_requests--;
            });
        }

        void override_transfer_evaluator::do_apply(const protocol::override_transfer_operation &o) {
            try {
                const asset_object &asset_type = db.get_asset(o.amount.symbol_name());
                STEEMIT_ASSERT(
                        asset_type.can_override(),
                        override_transfer_not_permitted,
                        "override_transfer not permitted for asset ${asset}",
                        ("asset", o.amount.symbol)
                );
                FC_ASSERT(asset_type.issuer == o.issuer);

                const account_object &from_account = db.get_account(o.from);
                const account_object &to_account = db.get_account(o.to);

                FC_ASSERT(db.is_authorized_asset(to_account, asset_type));
                FC_ASSERT(db.is_authorized_asset(from_account, asset_type));

                FC_ASSERT(db.get_balance(from_account, asset_type).amount >= o.amount.amount,
                        "", ("total_transfer", o.amount)("balance", db.get_balance(from_account, asset_type).amount));

                db.adjust_balance(db.get_account(o.from), -o.amount);
                db.adjust_balance(db.get_account(o.to), o.amount);
            } FC_CAPTURE_AND_RETHROW((o))
        }
    }
}