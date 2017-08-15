#ifndef GOLOS_DATABASE_WORKER_HPP
#define GOLOS_DATABASE_WORKER_HPP

#include <steemit/chain/dynamic_extension/worker.hpp>

namespace steemit {
    namespace chain {

        template<typename Database> class database_worker_t : public dynamic_extension::worker_t {
        public:

            database_worker_t(Database &db, const std::string &name) : worker_t(name), database(db) {
            }

            virtual ~database_worker_t() = default;

        protected:

            Database &database;
        };

    }
}
#endif //GOLOS_DATABASE_WORKER_HPP§
