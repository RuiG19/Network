#pragma once

#include "Server.hpp"

namespace network
{

class TcpServer : public Server<Tcp>
{
public:
    TcpServer(std::string ip_, uint16_t port_, std::function<void(uint16_t clientPort, std::unique_ptr<Payload> rxBuffer_)> handler_ = nullptr);

private:
    std::unique_ptr<Acceptor> acceptor;
    
    virtual void stop();
    virtual void start_up();

};

}
