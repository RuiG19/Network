#include "UdpServer.hpp"

namespace network
{

UdpServer::UdpServer(std::string ip_, uint16_t port_, std::function<void(uint16_t clientPort, std::unique_ptr<Payload> rxBuffer_)> handler_)
    : Server<Udp>(ip_, port_, handler_)
{

}

void UdpServer::stop()
{
    Server<Udp>::stop();
    
    try
    {
        acceptor->cancel();
        acceptor->close();

        io.stop();
    }
    catch(const std::exception& e)
    {
        LOG_ERROR_TAG("SERVER") << e.what();
    }
}


void UdpServer::start_up()
{
    LOG_DEBUG_TAG("SERVER") << "Starting SERVER thread";
    acceptor = std::make_unique<Acceptor>(io);

    try
    {
        LOG_DEBUG_TAG("SERVER") << "OPEN ip_v4 socket";
        acceptor->open(server_endpoint.protocol().v4());

        LOG_DEBUG_TAG("SERVER") << "SET_OPTION reuse_address(true)";
        acceptor->set_option(Acceptor::reuse_address(true));
    
        LOG_DEBUG_TAG("SERVER") << "BIND [" << server_endpoint.address().to_string() << ":" 
                              << server_endpoint.port() << "]";
        acceptor->bind(server_endpoint);

        LOG_DEBUG_TAG("SERVER") << "LISTEN start";
        acceptor->listen(boost::asio::socket_base::max_connections);    

        while(true)
        {
            if (acceptor->is_open())
            {
                LOG_DEBUG_TAG("SERVER") << "ACCEPTOR is open ... start waiting for a new connections";

                std::shared_ptr<Connection> connection = std::make_shared<Connection>();
            
                connection->getTxBuffer() = {'P','O','N','G'}; 
                connection->getRxBuffer() = Payload(4090); 
            
                acceptor->accept(connection->getSocket());

                LOG_DEBUG_TAG("SERVER") << "New connection accepted with Client(" << connection->getSocket().remote_endpoint().port() << ")";
                {
                    std::lock_guard<std::mutex> lock(rx_mutex);
                    connections.emplace(connection->getSocket().remote_endpoint().port(), connection);
                }

                LOG_DEBUG_TAG("SERVER") << "Setting Async Rx Callback for Client(" << connection->getSocket().remote_endpoint().port() << ")";
                connection->getSocket().async_receive(boost::asio::buffer(connection->getRxBuffer()), 
                    [=](const boost::system::error_code& ec, size_t bytes)
                    {
                        rx_callback(ec, bytes, connection);
                    });
            
                connection->start();

                LOG_DEBUG_TAG("SERVER") << " Starting io_context run";
                io.run();
            
            }
            else
            {
                LOG_ERROR_TAG("SERVER") << "ACCEPTOR is closed";
                exit (EXIT_FAILURE);
            }
        }

    }
    catch(const std::exception& e)
    {
        LOG_ERROR_TAG("SERVER") << e.what();
        exit (EXIT_FAILURE);
    }
}

}
