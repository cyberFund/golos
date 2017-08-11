#include <steemit/key_value/key_value_api.hpp>

namespace steemit {
    namespace key_value {

        namespace detail {

            class key_value_api_impl {
            public:
                key_value_api_impl(steemit::application::application &app)
                        : _app(app) {
                }

                first_key_value_object get_first_object_by_system(const std::string &system);

                first_key_value_object get_first_object_by_block_number(fc::uint128_t number);

                first_key_value_object get_first_object_by_hash(const std::string &hash);

                first_key_value_object get_first_object_by_ipfs_hash(const std::string &ipfs_hash);

                first_key_value_object get_first_object_by_account(const std::string &account);

                first_key_value_object get_first_object_by_time_range(const fc::time_point_sec &start, const fc::time_point_sec &end);

                steemit::application::application &_app;
            };

        } // detail

        key_value_api::key_value_api(const steemit::application::api_context &ctx) {
            my = std::make_shared<detail::key_value_api_impl>(ctx.app);
        }

        void key_value_api::on_api_startup() {
        }

        first_key_value_object detail::key_value_api_impl::get_first_object_by_system(const std::string &system) {
            return first_key_value_object();
        }

        first_key_value_object detail::key_value_api_impl::get_first_object_by_block_number(fc::uint128_t number) {
            return first_key_value_object();
        }

        first_key_value_object detail::key_value_api_impl::get_first_object_by_hash(const std::string &hash) {
            return first_key_value_object();
        }

        first_key_value_object detail::key_value_api_impl::get_first_object_by_ipfs_hash(const std::string &ipfs_hash) {
            return first_key_value_object();
        }

        first_key_value_object detail::key_value_api_impl::get_first_object_by_account(const std::string &account) {
            return first_key_value_object();
        }

        first_key_value_object detail::key_value_api_impl::get_first_object_by_time_range(
                const fc::time_point_sec &start, const fc::time_point_sec &end) {
            return first_key_value_object();
        }

        first_key_value_object key_value_api::get_first_object_by_system(const std::string &system) {
            return first_key_value_object();
        }

        first_key_value_object key_value_api::get_first_object_by_block_number(fc::uint128_t number) {
            return first_key_value_object();
        }

        first_key_value_object key_value_api::get_first_object_by_hash(const std::string &hash) {
            return first_key_value_object();
        }

        first_key_value_object key_value_api::get_first_object_by_ipfs_hash(const std::string &ipfs_hash) {
            return first_key_value_object();
        }

        first_key_value_object key_value_api::get_first_object_by_account(const std::string &account) {
            return first_key_value_object();
        }

        first_key_value_object key_value_api::get_first_object_by_time_range(const fc::time_point_sec &start,
                                                                             const fc::time_point_sec &end) {
            return first_key_value_object();
        }
    }
} // steemit::key_value