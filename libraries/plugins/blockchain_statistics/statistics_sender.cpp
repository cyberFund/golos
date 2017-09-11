#include <boost/asio.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/system/error_code.hpp>

#include <fc/exception/exception.hpp>
#include <boost/exception/all.hpp>
#include <fc/io/sstream.hpp>
#include <fc/log/logger.hpp>
#include <fc/io/json.hpp>

#include <string>
#include <cstdio>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <algorithm>
#include <queue>
#include <set>

#include "include/statistics_sender.hpp"

stat_client::stat_client() {
    QUEUE_ENABLED = false;
}

stat_client::stat_client(uint32_t default_port, uint32_t timeout) : default_port(default_port), sender_sleeping_time(timeout) {
    QUEUE_ENABLED = false;
}

stat_client::~stat_client() {
    QUEUE_ENABLED = false;
    if (!recipient_endpoint_set.empty()) {
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
    return !recipient_endpoint_set.empty();
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

                for (auto endpoint : recipient_endpoint_set) {
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
    // TODO add logs
    catch (const std::exception &ex)
    {
         std::cerr << ex.what() << std::endl;
    }
}

void stat_client::init() {
    QUEUE_ENABLED = true;
    cds::Initialize();
}

void stat_client::add_address(const std::string & address) {
    // Parsing "IP:PORT". If there is no port, then use Default one from configs.
    try
    {
        boost::asio::ip::udp::endpoint ep;
        boost::asio::ip::address ip;
        uint16_t port;        
        boost::system::error_code ec;

        auto pos = address.find(':');

        if (pos != std::string::npos) {
            ip = boost::asio::ip::address::from_string( address.substr( 0, pos ) , ec);
            port = boost::lexical_cast<uint16_t>( address.substr( pos + 1, address.size() ) );
        }
        else {
            ip = boost::asio::ip::address::from_string( address , ec);
            port = default_port;
        }
        
        if (ip.is_unspecified()) {
            // TODO something with exceptions and logs!
            ep = boost::asio::ip::udp::endpoint(boost::asio::ip::address::from_string("127.0.0.1"), port);
            recipient_endpoint_set.insert(ep);
        }
        else {
            ep = boost::asio::ip::udp::endpoint(ip, port);
            recipient_endpoint_set.insert(ep);
        }
    }
    FC_CAPTURE_AND_LOG(())
}

std::vector<std::string> stat_client::get_endpoint_string_vector() {
    std::vector<std::string> ep_vec;
    for (auto x : recipient_endpoint_set) {
        ep_vec.push_back( x.address().to_string() + ":" + std::to_string(x.port()));
    }
    return ep_vec;
}
