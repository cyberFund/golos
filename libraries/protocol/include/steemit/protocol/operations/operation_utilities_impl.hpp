#pragma once

#include <steemit/version/version_state.hpp>

#include <fc/static_variant.hpp>

namespace fc {
    std::string name_from_type(const std::string &type_name);

    struct from_operation {
        variant &var;

        from_operation(variant &dv) : var(dv) {
        }

        typedef void result_type;

        template<typename T>
        void operator()(const T &v) const {
            std::string name = name_from_type(fc::get_typename<T>::name());
            var = variant(std::make_pair(name.substr(0, name.find("<")), v));
        }
    };

    struct get_operation_name {
        string &name;

        get_operation_name(string &dv) : name(dv) {
        }

        typedef void result_type;

        template<typename T>
        void operator()(const T &v) const {
            name = name_from_type(fc::get_typename<T>::name());
        }
    };
}

namespace steemit {
    namespace protocol {

        struct operation_validate_visitor {
            typedef void result_type;

            template<typename T>
            void operator()(const T &v) const {
                v.validate();
            }
        };

        struct operation_get_required_auth_visitor {
            typedef void result_type;

            flat_set <protocol::account_name_type> &active;
            flat_set <protocol::account_name_type> &owner;
            flat_set <protocol::account_name_type> &posting;
            std::vector<authority> &other;

            operation_get_required_auth_visitor(flat_set <protocol::account_name_type> &a,
                                                flat_set <protocol::account_name_type> &own,
                                                flat_set <protocol::account_name_type> &post,
                                                std::vector<authority> &oth) : active(a), owner(own), posting(post),
                    other(oth) {
            }

            template<typename T>
            void operator()(const T &v) const {
                v.get_required_active_authorities(active);
                v.get_required_owner_authorities(owner);
                v.get_required_posting_authorities(posting);
                v.get_required_authorities(other);
            }
        };

    }
} // steemit::protocol
