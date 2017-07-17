#ifndef GOLOS_HARD_FORK_TRANSFORMER_HPP
#define GOLOS_HARD_FORK_TRANSFORMER_HPP

#include <unordered_map>
#include <vector>

#include <steemit/protocol/operations.hpp>

#include "forward.hpp"

namespace steemit {
    namespace chain {


        class hard_fork_transformer {
        public:
            using transformer_t = std::unordered_multimap<std::uint64_t, std::function<void()>>
            using d = std::pair<std::uint64_t, std::function<void()>>
            using chain = std::vector<std::function<void()>>;

            template<typename F>
            auto add(std::uint64_t hardfork, F &&f) -> void {
                transformer.emplace(hardfork, f);
            }

            auto get(std::uint64_t hardfork) const -> chain {
                chain tmp;

                for (d &i : transformer.equal_range(hardfork)) {
                    tmp.emplace(i.second);
                }

                return tmp;
            }

        private:
            transformer_t transformer;
        };


    }
}
#endif //GOLOS_HARD_FORK_TRANSFORMER_HPP
