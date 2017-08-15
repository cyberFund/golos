#ifndef GOLOS_WORKER_HPP
#define GOLOS_WORKER_HPP

#include <string>
#include <unordered_map>
#include <functional>
#include <boost/any.hpp>
#include <vector>

namespace steemit {
    namespace chain {
        namespace dynamic_extension {

            class abstract_worker_t {
            public:

                abstract_worker_t() = delete;

                abstract_worker_t(const abstract_worker_t &) = delete;

                abstract_worker_t &operator=(const abstract_worker_t &) = delete;

                explicit abstract_worker_t(const std::string &name);

                const std::string &name() const;

                template<typename ...Args> boost::any invoke(const std::string &command, Args... args) {
                    std::vector<boost::any> tmp = {(args)...};
                    return invoke_raw(command, tmp);
                }

            protected:
                virtual ~abstract_worker_t() = default;

                virtual boost::any invoke_raw(const std::string &, std::vector<boost::any>) = 0;

            private:
                std::string name_;
            };

            template<typename T> inline T cast(boost::any r) {
                return boost::any_cast<T>(r);
            }

            class worker_t : public abstract_worker_t {
            public:

                explicit worker_t(const std::string &name);

            protected:
                virtual ~worker_t() = default;

                template<int N, typename F> void add(const char (&key)[N], F &&f) {
                    dispather_.emplace(key, f);
                };

            private:
                boost::any invoke_raw(const std::string &, std::vector<boost::any>) override final;

                std::unordered_map<std::string, std::function<boost::any(std::vector<boost::any>)>> dispather_;
            };

        }
    }
}
#endif //GOLOS_WORKER_HPP
