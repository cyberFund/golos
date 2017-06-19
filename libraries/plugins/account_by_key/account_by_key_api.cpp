#include <steemit/account_by_key/account_by_key_api.hpp>
#include <fc/unique_ptr.hpp>

namespace steemit {
namespace account_by_key {

struct account_by_key_api::impl {
    impl(steemit::application::application &app) :app(app) {
    }

    ~impl() = default;

    vector<vector<account_name_type>> get_key_references(vector<public_key_type> &keys) const {
        vector<vector<account_name_type>> final_result;
        final_result.reserve(keys.size());

        const auto &key_idx = app.chain_database()->get_index<key_lookup_index>().indices().get<by_key>();

        for (auto &key : keys) {
            vector<account_name_type> result;
            auto lookup_itr = key_idx.lower_bound(key);

            while (lookup_itr != key_idx.end() &&
                   lookup_itr->key == key) {
                result.push_back(lookup_itr->account);
                ++lookup_itr;
            }

            final_result.emplace_back(std::move(result));
        }

        return final_result;
    }

    steemit::application::application &app;
};

account_by_key_api::account_by_key_api(const steemit::application::api_context &ctx) : pimpl(new impl(ctx.app)) {
}

void account_by_key_api::on_api_startup() {
}

vector<vector<account_name_type>> account_by_key_api::get_key_references(vector<public_key_type> keys) const {
    return pimpl->app.chain_database()->with_read_lock([&]() {
        return pimpl->get_key_references(keys);
    });
}

account_by_key_api::~account_by_key_api()=default;

}
} // steemit::account_by_key
