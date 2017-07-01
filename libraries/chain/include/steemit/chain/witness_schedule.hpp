#pragma once

namespace steemit {
    namespace chain {

        class database_basic;

        void update_witness_schedule(database_basic &db);

        void reset_virtual_schedule_time(database_basic &db);
    }
}