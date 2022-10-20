#include "UdpClient.hpp"

namespace network
{

UdpClient::UdpClient(std::string ip_, uint16_t port_, std::string server_ip_, uint16_t server_port_, std::function<void(std::unique_ptr<Payload> rxBuffer_)> handler_)
    : Client<Udp>(ip_, port_, server_ip_, server_port_, handler_)
{

}

void UdpClient::start_up()
{
    try
    {
        LOG_DEBUG_TAG(client_id) << "OPEN ip_v4 socket";
        server_socket->open(client_endpoint.protocol().v4());

        LOG_DEBUG_TAG(client_id) << "SET_OPTION reuse_address(true)";
        server_socket->set_option(Acceptor::reuse_address(true));

        LOG_DEBUG_TAG(client_id) << "BIND [" << client_endpoint.address().to_string() << ":" 
                              << client_endpoint.port() << "]";
        server_socket->bind(client_endpoint);

        LOG_DEBUG_TAG(client_id) << "CONNECT TO [" << server_endpoint.address().to_string() << ":" 
                              << server_endpoint.port() << "]";
        server_socket->connect(server_endpoint);

        LOG_DEBUG_TAG(client_id) << "Setting Async Rx Callback";
        server_socket->async_receive(boost::asio::buffer(rx_buffer), [=](const boost::system::error_code& ec, size_t bytes){this->rx_callback(ec, bytes); });

        LOG_DEBUG_TAG(client_id) << "Sending PING to Server(" << server_socket->remote_endpoint().port() << ")";
        server_socket->send(boost::asio::buffer(tx_buffer));

        LOG_DEBUG_TAG(client_id) << "Starting io_context run";
        io.run();
    }
    catch(const std::exception& e)
    {
        LOG_ERROR_TAG(client_id) << e.what();
        exit (EXIT_FAILURE);
    }
}


}
