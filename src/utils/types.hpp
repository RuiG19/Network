#pragma once

#include <vector>
#include <boost/asio.hpp>


namespace network
{

// STL types
typedef std::vector<uint8_t> Payload;

// Boost types
typedef boost::asio::io_context Context;

typedef boost::asio::ip::tcp Tcp;
typedef boost::asio::ip::udp Udp;

template<typename Protocol>
using Endpoint = typename Protocol::endpoint;

template<typename Protocol>
using Socket = typename Protocol::socket;

typedef Tcp::acceptor Acceptor;

}
