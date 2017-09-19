#include <steemit/protocol/operations/steem_operations.hpp>

#include <fc/io/json.hpp>

namespace steemit {
    namespace protocol {

        /// TODO: after the hardfork, we can rename this method validate_permlink because it is strictily less restrictive than before
        ///  Issue #56 contains the justificiation for allowing any UTF-8 string to serve as a permlink, content will be grouped by tags
        ///  going forward.
        inline void validate_permlink(const string &permlink) {
            FC_ASSERT(permlink.size() < STEEMIT_MAX_PERMLINK_LENGTH, "permlink is too long");
            FC_ASSERT(fc::is_utf8(permlink), "permlink not formatted in UTF8");
        }

        inline void validate_account_name(const string &name) {
            FC_ASSERT(is_valid_account_name(name), "Account name ${n} is invalid", ("n", name));
        }

        void challenge_authority_operation::validate() const {
            validate_account_name(challenger);
            validate_account_name(challenged);
            FC_ASSERT(challenged != challenger, "cannot challenge yourself");
        }

        void prove_authority_operation::validate() const {
            validate_account_name(challenged);
        }

        void vote_operation::validate() const {
            validate_account_name(voter);
            validate_account_name(author);
            FC_ASSERT(abs(weight) <= STEEMIT_100_PERCENT, "Weight is not a STEEMIT percentage");
            validate_permlink(permlink);
        }

        void withdraw_vesting_operation::validate() const {
            validate_account_name(account);
            FC_ASSERT(vesting_shares.symbol_type_value() == VESTS_SYMBOL, "Amount must be VESTS");
        }

        void set_withdraw_vesting_route_operation::validate() const {
            validate_account_name(from_account);
            validate_account_name(to_account);
            FC_ASSERT(0 <= percent && percent <= STEEMIT_100_PERCENT, "Percent must be valid steemit percent");
        }

        void witness_update_operation::validate() const {
            validate_account_name(owner);
            FC_ASSERT(url.size() > 0, "URL size must be greater than 0");
            FC_ASSERT(fc::is_utf8(url), "URL is not valid UTF8");
            FC_ASSERT(fee >= asset(0, STEEM_SYMBOL_NAME), "Fee cannot be negative");
            props.validate();
        }

        void account_witness_vote_operation::validate() const {
            validate_account_name(account);
            validate_account_name(witness);
        }

        void account_witness_proxy_operation::validate() const {
            validate_account_name(account);
            if (proxy.size()) {
                validate_account_name(proxy);
            }
            FC_ASSERT(proxy != account, "Cannot proxy to self");
        }

        void custom_operation::validate() const {
            /// required auth accounts are the ones whose bandwidth is consumed
            FC_ASSERT(required_auths.size() > 0, "at least on account must be specified");
        }

        void custom_json_operation::validate() const {
            /// required auth accounts are the ones whose bandwidth is consumed
            FC_ASSERT((required_auths.size() + required_posting_auths.size()) > 0,
                      "at least on account must be specified");
            FC_ASSERT(id.size() <= 32, "id is too long");
            FC_ASSERT(fc::is_utf8(json), "JSON Metadata not formatted in UTF8");
            FC_ASSERT(fc::json::is_valid(json), "JSON Metadata not valid JSON");
        }

        void custom_binary_operation::validate() const {
            /// required auth accounts are the ones whose bandwidth is consumed
            FC_ASSERT((required_owner_auths.size() + required_active_auths.size() + required_posting_auths.size()) > 0,
                      "at least on account must be specified");
            FC_ASSERT(id.size() <= 32, "id is too long");
            for (const auto &a : required_auths) {
                a.validate();
            }
        }


        fc::sha256 pow_operation::work_input() const {
            auto hash = fc::sha256::hash(block_id);
            hash._hash[0] = nonce;
            return fc::sha256::hash(hash);
        }

        void pow_operation::validate() const {
            props.validate();
            validate_account_name(worker_account);
            FC_ASSERT(work_input() == work.input, "Determninistic input does not match recorded input");
            work.validate();
        }

        struct pow2_operation_validate_visitor {
            typedef void result_type;

            template<typename PowType>
            void operator()(const PowType &pow) const {
                pow.validate();
            }
        };

        void pow2_operation::validate() const {
            props.validate();
            work.visit(pow2_operation_validate_visitor());
        }

        struct pow2_operation_get_required_active_visitor {
            typedef void result_type;

            pow2_operation_get_required_active_visitor(flat_set<account_name_type> &required_active) : _required_active(
                    required_active) {
            }

            template<typename PowType>
            void operator()(const PowType &work) const {
                _required_active.insert(work.input.worker_account);
            }

            flat_set<account_name_type> &_required_active;
        };

        void pow2_operation::get_required_active_authorities(flat_set<account_name_type> &a) const {
            if (!new_owner_key) {
                pow2_operation_get_required_active_visitor vtor(a);
                work.visit(vtor);
            }
        }

        void pow::create(const fc::ecc::private_key &w, const digest_type &i) {
            input = i;
            signature = w.sign_compact(input, false);

            auto sig_hash = fc::sha256::hash(signature);
            public_key_type recover = fc::ecc::public_key(signature, sig_hash, false);

            work = fc::sha256::hash(recover);
        }

        void pow2::create(const block_id_type &prev, const account_name_type &account_name, uint64_t n) {
            input.worker_account = account_name;
            input.prev_block = prev;
            input.nonce = n;

            auto prv_key = fc::sha256::hash(input);
            auto input = fc::sha256::hash(prv_key);
            auto signature = fc::ecc::private_key::regenerate(prv_key).sign_compact(input);

            auto sig_hash = fc::sha256::hash(signature);
            public_key_type recover = fc::ecc::public_key(signature, sig_hash);

            fc::sha256 work = fc::sha256::hash(std::make_pair(input, recover));
            pow_summary = work.approx_log_32();
        }

        void equihash_pow::create(const block_id_type &recent_block, const account_name_type &account_name,
                                  uint32_t nonce) {
            input.worker_account = account_name;
            input.prev_block = recent_block;
            input.nonce = nonce;

            auto seed = fc::sha256::hash(input);
            proof = fc::equihash::proof::hash(STEEMIT_EQUIHASH_N, STEEMIT_EQUIHASH_K, seed);
            pow_summary = fc::sha256::hash(proof.inputs).approx_log_32();
        }

        void pow::validate() const {
            FC_ASSERT(work != fc::sha256());
            FC_ASSERT(public_key_type(fc::ecc::public_key(signature, input, false)) == worker);
            auto sig_hash = fc::sha256::hash(signature);
            public_key_type recover = fc::ecc::public_key(signature, sig_hash, false);
            FC_ASSERT(work == fc::sha256::hash(recover));
        }

        void pow2::validate() const {
            validate_account_name(input.worker_account);
            pow2 tmp;
            tmp.create(input.prev_block, input.worker_account, input.nonce);
            FC_ASSERT(pow_summary == tmp.pow_summary, "reported work does not match calculated work");
        }

        void equihash_pow::validate() const {
            validate_account_name(input.worker_account);
            auto seed = fc::sha256::hash(input);
            FC_ASSERT(proof.n == STEEMIT_EQUIHASH_N, "proof of work 'n' value is incorrect");
            FC_ASSERT(proof.k == STEEMIT_EQUIHASH_K, "proof of work 'k' value is incorrect");
            FC_ASSERT(proof.seed == seed, "proof of work seed does not match expected seed");
            FC_ASSERT(proof.is_valid(), "proof of work is not a solution",
                      ("block_id", input.prev_block)("worker_account", input.worker_account)("nonce", input.nonce));
            FC_ASSERT(pow_summary == fc::sha256::hash(proof.inputs).approx_log_32());
        }

        void feed_publish_operation::validate() const {
            validate_account_name(publisher);
            FC_ASSERT(
                    (exchange_rate.base.symbol == STEEM_SYMBOL_NAME && exchange_rate.quote.symbol == SBD_SYMBOL_NAME) ||
                    (exchange_rate.base.symbol == SBD_SYMBOL_NAME && exchange_rate.quote.symbol == STEEM_SYMBOL_NAME),
                    "Price feed must be a STEEM/SBD price");
            exchange_rate.validate();
        }

        void report_over_production_operation::validate() const {
            validate_account_name(reporter);
            validate_account_name(first_block.witness);
            FC_ASSERT(first_block.witness == second_block.witness);
            FC_ASSERT(first_block.timestamp == second_block.timestamp);
            FC_ASSERT(first_block.signee() == second_block.signee());
            FC_ASSERT(first_block.id() != second_block.id());
        }

        void request_account_recovery_operation::validate() const {
            validate_account_name(recovery_account);
            validate_account_name(account_to_recover);
            new_owner_authority.validate();
        }

        void recover_account_operation::validate() const {
            validate_account_name(account_to_recover);
            FC_ASSERT(!(new_owner_authority == recent_owner_authority),
                      "Cannot set new owner authority to the recent owner authority");
            FC_ASSERT(!new_owner_authority.is_impossible(), "new owner authority cannot be impossible");
            FC_ASSERT(!recent_owner_authority.is_impossible(), "recent owner authority cannot be impossible");
            FC_ASSERT(new_owner_authority.weight_threshold, "new owner authority cannot be trivial");
            new_owner_authority.validate();
            recent_owner_authority.validate();
        }

        void change_recovery_account_operation::validate() const {
            validate_account_name(account_to_recover);
            validate_account_name(new_recovery_account);
        }

        void decline_voting_rights_operation::validate() const {
            validate_account_name(account);
        }

        void reset_account_operation::validate() const {
            validate_account_name(reset_account);
            validate_account_name(account_to_reset);
            FC_ASSERT(!new_owner_authority.is_impossible(), "new owner authority cannot be impossible");
            FC_ASSERT(new_owner_authority.weight_threshold, "new owner authority cannot be trivial");
            new_owner_authority.validate();
        }

        void set_reset_account_operation::validate() const {
            validate_account_name(account);
            if (current_reset_account.size()) {
                validate_account_name(current_reset_account);
            }
            validate_account_name(reset_account);
            FC_ASSERT(current_reset_account != reset_account, "new reset account cannot be current reset account");
        }

        void delegate_vesting_shares_operation::validate() const {
            validate_account_name(delegator);
            validate_account_name(delegatee);
            FC_ASSERT(delegator != delegatee, "You cannot delegate VESTS to yourself");
            FC_ASSERT(vesting_shares.symbol_type_value() == VESTS_SYMBOL, "Delegation must be VESTS");
            FC_ASSERT(vesting_shares >= asset(0, VESTS_SYMBOL), "Delegation cannot be negative");
        }
    }
} // steemit::protocol
