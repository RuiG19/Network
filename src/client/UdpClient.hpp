#pragma once

#include "Client.hpp"

namespace network
{

class UdpClient : public Client<Udp>
{
public:
    UdpClient(std::string ip_, uint16_t port_, std::string server_ip_, uint16_t server_port_, std::function<void(std::unique_ptr<Payload> rxBuffer_)> handler_ = nullptr);

private:
    virtual void start_up();

};

}
