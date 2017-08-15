#include <steemit/chain/dynamic_extension/worker.hpp>
#include <iostream>

namespace steemit {
    namespace chain {
        namespace dynamic_extension {
            boost::any worker_t::invoke_raw(const std::string &command, std::vector<boost::any> args) {
                auto it = dispatcher.find(command);
                if (it != dispatcher.end()) {
                    return dispatcher.at(command)(args);
                } else {
                    std::cerr << "Trash" << std::endl;
                }

            }

            worker_t::worker_t(const std::string &name) : abstract_worker_t(name) {
            }

            abstract_worker_t::abstract_worker_t(const std::string &name) : name_(name) {
            }

            const std::string &abstract_worker_t::name() const {
                return name_;
            }
        }
    }
}
