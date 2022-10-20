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
class Server
{
public:
    virtual ~Server();

    void start();
    virtual void stop();
    std::future_status status() const;

    void send(uint16_t clientPort, std::unique_ptr<Payload> txBuffer_);

protected:
    Server(std::string ip_, uint16_t port_, std::function<void(uint16_t clientPort, std::unique_ptr<Payload> rxBuffer_)> handler_ = nullptr);
    
    class Connection
    {
        public:
        Connection();
        ~Connection();

        void start();
        void stop();

        Socket<Protocol>& getSocket(); 
        Payload& getRxBuffer();
        Payload& getTxBuffer();

        private:
        Context context_io;
        std::thread _thread;
        std::shared_ptr<Socket<Protocol>> socket;
        Payload rx_buffer; 
        Payload tx_buffer; 
    };

    Endpoint<Protocol> server_endpoint;
    Context io;
    // std::unique_ptr<Acceptor> acceptor;

    std::map<uint16_t, std::shared_ptr<Connection>> connections;
    std::mutex rx_mutex;

    std::function<void(uint16_t clientPort, std::unique_ptr<Payload> rxBuffer_)> handler;

    virtual void start_up() = 0;
    void rx_callback(const boost::system::error_code& ec, size_t bytes, std::shared_ptr<Connection> client_connection);

private:
    std::future<void> status_future;

};

}
