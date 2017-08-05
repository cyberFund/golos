#ifndef GOLOS_DATABASE_HPP
#define GOLOS_DATABASE_HPP

#include <steemit/chain/evaluators/evaluator_registry.hpp>
#include <steemit/chain/index.hpp>

#include "database_police.hpp"
#include "hard_fork_transformer.hpp"
#include <steemit/chain/dynamic_extension/worker_storage.hpp>


namespace steemit {
    namespace chain {
        class database final : public database_tag {
        public:
            database(dynamic_extension::worker_storage&& storage, hard_fork_transformer &&hft) : database_tag(std::move(storage), std::move(hft)) {}
            ~database() = default;

        protected:
            void initialize_indexes();

            void initialize_evaluators();

            void apply_operation(const operation &op);

            evaluator_registry<operation> evaluator_registry_;
        };


        shared_ptr<database> make_database();


    }
}
#endif //GOLOS_DATABASE_HPP
