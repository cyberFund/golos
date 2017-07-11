#include <boost/asio.hpp>
#include <fc/network/ip.hpp>
// boost/asio/ip/detail/endpoint.hpp

#include <string>
#include <cstdio>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <algorithm>
#include <queue>

#include "include/statistics_sender.hpp"

stat_client::stat_client() {
    QUEUE_ENABLED = false;
}

stat_client::stat_client(uint32_t br_port, uint32_t timeout) : port(br_port),
    sender_sleeping_time(timeout) {
}

stat_client::~stat_client() {
    QUEUE_ENABLED = false;
    if (!recipient_endpoint_vec.empty()) {
        sender_thread.join();
    }
    cds::Terminate();
}


void stat_client::stop() {
    QUEUE_ENABLED = false;
}

void stat_client::push(const std::string & item) {
    stat_q.enqueue(item);
}

bool stat_client::can_start() {
    return !recipient_endpoint_vec.empty();
}

void stat_client::start() {
    init();
    // Lambda which implements the data sending loop
    auto run_broadcast_loop = [&]() {
        boost::asio::io_service io_service;

        boost::asio::ip::udp::socket socket(io_service, boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), 0));
        socket.set_option(boost::asio::socket_base::broadcast(true));


        cds::threading::Manager::attachThread();

        while (QUEUE_ENABLED.load() || !stat_q.empty()) {
            while (!stat_q.empty()) {                
                std::string tmp_s;

                stat_q.dequeue(tmp_s);

                for (auto endpoint : recipient_endpoint_vec) {
                    socket.send_to(boost::asio::buffer(tmp_s), endpoint);
                }
            }
            std::this_thread::sleep_for(std::chrono::seconds(sender_sleeping_time));
        }
                
        cds::threading::Manager::detachThread();
    };
    try
    {   
        if (can_start()) {
            std::thread sending_thr(run_broadcast_loop);
            sender_thread = std::move(sending_thr);
        }
    }
    catch (const std::exception &ex)
    {
         std::cerr << ex.what() << std::endl;
    }
}

void stat_client::add_address(const std::string & address) {
    /*
        Using correct parsing from fc endpoint.
        Then make asio udp endpoint. Then push it to the vector.
    */
    auto tmp_ep = fc::ip::endpoint::from_string(address);

    auto tmp_ip = fc::string(tmp_ep.get_address());
    uint32_t tmp_port = tmp_ep.port();

    boost::asio::ip::udp::endpoint boost_endpoint(boost::asio::ip::address::from_string(tmp_ip), tmp_port);
    recipient_endpoint_vec.push_back(boost_endpoint);
}

void stat_client::init() {
    QUEUE_ENABLED = true;
    cds::Initialize();
}
