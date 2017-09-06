#pragma once

// Libcds initialize and terminate
#include <cds/init.h>
// Libcds implementation of FCQueue
#include <cds/container/fcqueue.h>
#include <cds/threading/model.h>
#include <thread>
#include <vector>
#include <atomic>

#include <steemit/chain/steem_object_types.hpp>

#include <fc/api.hpp>
#include <fc/uint128.hpp>
#include <boost/asio/ip/udp.hpp>
// #include <fc/asio.hpp>

using namespace steemit::chain;

class stat_client {    

public:
    stat_client();
    stat_client(uint32_t default_port, uint32_t timeout);
    ~stat_client();

    // terminates sending loop
    void stop();

    // pushes a string to the stat_q
    void push(const std::string & item);

    // Checks is recipient_ip_vec empty of not. 
    bool can_start();

    // initializing and runs data sending loop in a new thread
    void start();
    
    // adds address to _recipient_ip_vec.
    void add_address(const std::string & address);

    /// returns statistics recievers endpoints
    std::vector<std::string> get_endpoint_string_vector();

private:
    // Flag which indicates is pushing data enabled or not
    std::atomic_bool QUEUE_ENABLED;
    // The thread which contains data sending loop
    std::thread sender_thread;
    // Lock-free FCQueue (not intrusive)
    cds::container::FCQueue<std::string, std::queue<std::string>, cds::container::fcqueue::traits> stat_q;
    // Stat sender will send data to all endpoints from recipient_endpoint_set
    std::set<boost::asio::ip::udp::endpoint> recipient_endpoint_set;
    // Port for asio broadcasting 
    uint32_t default_port;
    // Timeout in seconds
    uint32_t sender_sleeping_time;

    void init();
};
