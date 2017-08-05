#ifndef GOLOS_DATABASE_WORKER_HPP
#define GOLOS_DATABASE_WORKER_HPP

#include <steemit/chain/dynamic_extension/worker.hpp>

namespace steemit {
    namespace chain {

        template<typename DataBase>
        class database_worker_t : public  dynamic_extension::worker_t {
        public:

            database_worker_t(DataBase& db,const std::string& name):worker_t(name),database(db){}

            virtual ~database_worker_t() = default;

        protected:

            DataBase& database;
        };

    }
}
#endif //GOLOS_DATABASE_WORKER_HPP§
