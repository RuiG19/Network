#include "Server.hpp"

namespace network
{

template<typename Protocol>
Server<Protocol>::Server(std::string ip_, uint16_t port_, std::function<void(uint16_t clientPort, std::unique_ptr<Payload> rxBuffer_)> handler_) : handler(handler_)
{
    server_endpoint = Endpoint<Protocol>(boost::asio::ip::make_address_v4(ip_), port_);
}

template<typename Protocol>
Server<Protocol>::~Server()
{
    stop();
}

template<typename Protocol>
void Server<Protocol>::start()
{
    status_future = std::async(std::launch::async, [=](){ this->start_up(); });
}

template<typename Protocol>
void Server<Protocol>::stop()
{
    connections.clear();

    // try
    // {
    //     acceptor->cancel();
    //     acceptor->close();

    //     io.stop();
    // }
    // catch(const std::exception& e)
    // {
    //     LOG_ERROR_TAG("SERVER") << e.what();
    // }
}

template<typename Protocol>
std::future_status Server<Protocol>::status() const
{
    return status_future.wait_for(std::chrono::milliseconds(0));
}

template<typename Protocol>
void Server<Protocol>::send(uint16_t clientPort, std::unique_ptr<Payload> txBuffer_)
{
    if(txBuffer_->size() == 0)
    {
        LOG_WARNING_TAG("SERVER") << "Empty Payload, will ignore!";
        return;
    }

    if(connections.find(clientPort) != connections.end())
    {
        LOG_DEBUG_TAG("SERVER") << "Sending Payload with " << txBuffer_->size() << " bytes to Client(" << clientPort << ")";
        connections.at(clientPort)->getTxBuffer() = *txBuffer_;
        connections.at(clientPort)->getSocket().send(boost::asio::buffer(connections.at(clientPort)->getTxBuffer()));
    }
    else
    {
        LOG_WARNING_TAG("SERVER") << "No Connection available to client with port " << clientPort;   
    }
}

template<typename Protocol>
void Server<Protocol>::rx_callback(const boost::system::error_code& ec, size_t bytes, std::shared_ptr<Connection> client_connection)
{
    std::lock_guard<std::mutex> lock(rx_mutex);

    
    uint16_t clientPort = client_connection->getSocket().remote_endpoint().port();
    LOG_DEBUG_TAG("SERVER") << "Got something from Client(" << clientPort << ")";
    
    if(ec)
    {
        LOG_WARNING_TAG("SERVER") << "Erro code: " << ec.message();
        if(ec == boost::asio::error::eof)
        {
            LOG_DEBUG_TAG("SERVER") << "Client(" << clientPort << ") closed the connection!";
            connections.erase(clientPort);
        }
        return;
    }

    LOG_DEBUG_TAG("SERVER") << "Received " << bytes << " bytes";
    if(bytes)
    {
        std::string payload(client_connection->getRxBuffer().begin(), client_connection->getRxBuffer().end());
        LOG_DEBUG_TAG("SERVER") << "Rx Payload: " << payload;

        if(handler)
        {
            std::unique_ptr<Payload> rxPayload = std::make_unique<Payload>(client_connection->getRxBuffer());
            handler(clientPort, std::move(rxPayload));
        }
        else
        {
            // if no handler is defined simply Pong the client (use as default impl - maybe be comment out this section later)
            LOG_DEBUG_TAG("SERVER") << "Sending PONG to Client(" << clientPort << ")";
            client_connection->getSocket().send(boost::asio::buffer(client_connection->getTxBuffer()));
        }

        LOG_DEBUG_TAG("SERVER") << "Setting Async Rx Callback for Client(" << clientPort << ")";
        client_connection->getSocket().async_receive(boost::asio::buffer(client_connection->getRxBuffer()), 
            [=](const boost::system::error_code& ec, size_t bytes)
            {
                rx_callback(ec, bytes, client_connection);
            });
    }

}

template<typename Protocol>
Server<Protocol>::Connection::Connection()
    : socket(std::make_shared<Socket<Protocol>>(context_io))
{

}

template<typename Protocol>
Server<Protocol>::Connection::~Connection()
{
    stop();
}

template<typename Protocol>
void Server<Protocol>::Connection::start()
{
    _thread = std::thread([=](){ context_io.run(); });
}

template<typename Protocol>
void Server<Protocol>::Connection::stop()
{
    rx_buffer.resize(0);
    tx_buffer.resize(0);

    try
    {
        uint16_t port = socket->remote_endpoint().port();
        LOG_DEBUG_TAG("SERVER") << "Deleting endpoint: " << port;
        socket->cancel();
        socket->close();
        
        context_io.stop();
        LOG_DEBUG_TAG("SERVER") << "Endpoint(" << port << ") context_io stopped: " << std::boolalpha << context_io.stopped();
        _thread.detach(); // cannot use join since this is most likely being executted in the same context (Server<Protocol>::rx_callback() uses this thread context)
        LOG_DEBUG_TAG("SERVER") << "Endpoint(" << port << ") thread detached";
    }
    catch(const std::exception& e)
    {
        LOG_ERROR_TAG("SERVER") << e.what();
    }
}

template<typename Protocol>
Socket<Protocol>& Server<Protocol>::Connection::getSocket()
{
    return *socket;
}

template<typename Protocol>
Payload& Server<Protocol>::Connection::getRxBuffer()
{
    return rx_buffer;
}

template<typename Protocol>
Payload& Server<Protocol>::Connection::getTxBuffer()
{
    return tx_buffer;
}


template class Server<Tcp>;
template class Server<Udp>;

}
