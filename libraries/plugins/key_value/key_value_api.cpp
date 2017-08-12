#include <steemit/key_value/key_value_api.hpp>

namespace steemit {
    namespace key_value {

        namespace detail {

            class key_value_api_impl {
            public:
                key_value_api_impl(steemit::application::application &app) : _app(app) {
                }

                vector <first_key_value_object> get_first_object_by_system(const std::string &system);

                vector <first_key_value_object> get_first_object_by_block_number(fc::uint128_t number);

                vector <first_key_value_object> get_first_object_by_hash(const std::string &hash);

                vector <first_key_value_object> get_first_object_by_ipfs_hash(const std::string &ipfs_hash);

                vector <first_key_value_object> get_first_object_by_account(const std::string &account);

                vector<first_key_value_object> get_first_object_by_time_range(const fc::time_point_sec &start,
                                                                              const fc::time_point_sec &end);

                steemit::application::application &_app;
            };

        } // detail

        key_value_api::key_value_api(const steemit::application::api_context &ctx) {
            my = std::make_shared<detail::key_value_api_impl>(ctx.app);
        }

        void key_value_api::on_api_startup() {
        }

        vector <first_key_value_object> detail::key_value_api_impl::get_first_object_by_system(
                const std::string &system) {
            vector<first_key_value_object> result;
            const auto &index = _app.chain_database()->get_index<key_value_first_index>().indicies().get<by_system>();
            auto itr = index.lower_bound(system);

            while (itr != index.end() && itr->system == system) {
                result.emplace_back(*itr);
                ++itr;
            }

            return result;
        }

        vector <first_key_value_object> detail::key_value_api_impl::get_first_object_by_block_number(
                fc::uint128_t number) {
            vector<first_key_value_object> result;
            const auto &index = _app.chain_database()->get_index<key_value_first_index>().indicies().get<
                    by_block_number>();
            auto itr = index.lower_bound(number);

            while (itr != index.end() && itr->block_number == number) {
                result.emplace_back(*itr);
                ++itr;
            }

            return result;
        }

        vector <first_key_value_object> detail::key_value_api_impl::get_first_object_by_hash(const std::string &hash) {
            vector<first_key_value_object> result;
            const auto &index = _app.chain_database()->get_index<key_value_first_index>().indicies().get<
                    by_block_hash>();
            auto itr = index.lower_bound(hash);

            while (itr != index.end() && itr->block_hash == hash) {
                result.emplace_back(*itr);
                ++itr;
            }

            return result;
        }

        vector <first_key_value_object> detail::key_value_api_impl::get_first_object_by_ipfs_hash(
                const std::string &ipfs_hash) {
            vector<first_key_value_object> result;
            const auto &index = _app.chain_database()->get_index<key_value_first_index>().indicies().get<
                    by_ipfs_hash>();
            auto itr = index.lower_bound(ipfs_hash);

            while (itr != index.end() && itr->ipfs_hash_link == ipfs_hash) {
                result.emplace_back(*itr);
                ++itr;
            }

            return result;
        }

        vector <first_key_value_object> detail::key_value_api_impl::get_first_object_by_account(
                const std::string &account) {
            vector<first_key_value_object> result;
            const auto &index = _app.chain_database()->get_index<key_value_first_index>().indicies().get<by_owner>();
            auto itr = index.lower_bound(account);

            while (itr != index.end() && itr->owner == account) {
                result.emplace_back(*itr);
                ++itr;
            }

            return result;
        }

        vector<first_key_value_object> detail::key_value_api_impl::get_first_object_by_time_range(
                const fc::time_point_sec &start, const fc::time_point_sec &end) {
            vector<first_key_value_object> result;
            const auto &index = _app.chain_database()->get_index<key_value_first_index>().indicies().get<
                    by_timestamp>();
            auto itr = index.lower_bound(start);

            while (itr != index.end() && itr->timestamp < end) {
                result.emplace_back(*itr);
                ++itr;
            }

            return result;
        }

        vector <first_key_value_object> key_value_api::get_first_object_by_system(const std::string &system) {
            return my->get_first_object_by_system(system);
        }

        vector <first_key_value_object> key_value_api::get_first_object_by_block_number(fc::uint128_t number) {
            return my->get_first_object_by_block_number(number);
        }

        vector <first_key_value_object> key_value_api::get_first_object_by_hash(const std::string &hash) {
            return my->get_first_object_by_hash(hash);
        }

        vector <first_key_value_object> key_value_api::get_first_object_by_ipfs_hash(const std::string &ipfs_hash) {
            return my->get_first_object_by_ipfs_hash(ipfs_hash);
        }

        vector <first_key_value_object> key_value_api::get_first_object_by_account(const std::string &account) {
            return my->get_first_object_by_account(account);
        }

        vector<first_key_value_object> key_value_api::get_first_object_by_time_range(const fc::time_point_sec &start,
                                                                                     const fc::time_point_sec &end) {
            return my->get_first_object_by_time_range(start, end);
        }
    }
} // steemit::key_value