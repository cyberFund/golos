#ifndef GOLOS_WORKER_HPP
#define GOLOS_WORKER_HPP

#include <string>
#include <unordered_map>
#include <functional>
#include <boost/any.hpp>
#include "view.hpp"


namespace steemit {
    namespace chain {
        class worker_t {
        public:
            template<int N>
            worker_t(char (&data)[N]):name(data) {}

            ~worker() = default;

            view_t view() const {
                return view_t(const_cast<worker_t *>(this));
            }

            template<typename T>
            T invoke(const std::string &comanda, boost::any args) {
                return boost::any_cast<T>(invoke(comanda, args));
            }

            boost::any invoke(const std::string &comanda, boost::any args) {
                return dispather_.at(comanda)(args);
            }

            const std::string &name() const {
                return name_;
            }

        protected:
            std::string name_;

            template<typename F>
            void add(std::string key, F &&f) {
                dispather_.emplace(key, f);
            };
        private:
            std::unordered_map<std::string, std::function<boost::any(boost::any)>> dispather_;

        };
    }
}
#endif //GOLOS_WORKER_HPP
