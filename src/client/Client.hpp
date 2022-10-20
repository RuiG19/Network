#pragma once

#include <iostream>
#include <thread>
#include <future>
#include <boost/asio.hpp>
#include "types.hpp"
#include "Logger.hpp"

namespace network
{

template<typename Protocol>
class Client
{
public:
    virtual ~Client();

    void start();
    void stop();
    std::future_status status() const;
    uint16_t getId() const;

    void send(std::unique_ptr<Payload> txBuffer_);

protected:
    Client(std::string ip_, uint16_t port_, std::string server_ip_, uint16_t server_port_, std::function<void(std::unique_ptr<Payload> rxBuffer_)> handler_ = nullptr);

    const std::string client_id;

    Context io;
    Endpoint<Protocol> client_endpoint;
    Endpoint<Protocol> server_endpoint;

    Payload rx_buffer; 
    Payload tx_buffer; 

    std::shared_ptr<Socket<Protocol>> server_socket;

    std::function<void(std::unique_ptr<Payload> rxBuffer_)> handler;

    virtual void start_up() = 0;
    void rx_callback(const boost::system::error_code& ec, size_t bytes);

private:
    static uint16_t _id_generator;
    uint16_t id;
    
    std::future<void> status_future;
};

}

