#include "Client.hpp"

namespace network
{

template<typename Protocol>
uint16_t Client<Protocol>::_id_generator = 0;

template<typename Protocol>
Client<Protocol>::Client(std::string ip_, uint16_t port_, std::string server_ip_, uint16_t server_port_, std::function<void(std::unique_ptr<Payload> rxBuffer_)> handler_) 
              : id(++_id_generator), client_id("CLIENT_" + std::to_string(id)), handler(handler_)
{
    client_endpoint = Endpoint<Protocol>(boost::asio::ip::make_address_v4(ip_), port_);
    server_endpoint = Endpoint<Protocol>(boost::asio::ip::make_address_v4(server_ip_), server_port_);
    tx_buffer = {'P','I','N','G'}; 
    rx_buffer = Payload(4090); 
}

template<typename Protocol>
Client<Protocol>::~Client()
{
    stop();
}

template<typename Protocol>
void Client<Protocol>::start()
{
    LOG_DEBUG_TAG(client_id) << "Starting CLIENT thread";
    server_socket = std::make_shared<Socket<Protocol>>(io);
    status_future = std::async(std::launch::async, [=](){ this->start_up(); });
}

template<typename Protocol>
void Client<Protocol>::stop()
{
    LOG_DEBUG_TAG(client_id) << "Stopping CLIENT thread";

    rx_buffer.resize(0);
    tx_buffer.resize(0);

    try
    {
        server_socket->cancel();
        server_socket->close();

        io.stop();
    }
    catch(const std::exception& e)
    {
        LOG_ERROR_TAG(client_id) << e.what();
    }
}

template<typename Protocol>
uint16_t Client<Protocol>::getId() const
{
    return id;
}

template<typename Protocol>
std::future_status Client<Protocol>::status() const
{
    return status_future.wait_for(std::chrono::milliseconds(0));
}

template<typename Protocol>
void Client<Protocol>::send(std::unique_ptr<Payload> txBuffer_)
{
    if(txBuffer_->size() == 0)
    {
        LOG_WARNING_TAG(client_id) << "Empty Payload, will ignore!";
        return;
    }

    LOG_DEBUG_TAG(client_id) << "Sending Payload with " << txBuffer_->size() << " bytes to Server";
    tx_buffer = *txBuffer_;
    try
    {
        server_socket->send(boost::asio::buffer(tx_buffer));
    }
    catch(const std::exception& e)
    {
        LOG_ERROR_TAG(client_id) << e.what();
    }    
}

// void Client<Protocol>::start_up()
// {
//     LOG_DEBUG_TAG(client_id) << "Starting CLIENT thread";

//     server_socket = std::make_shared<Socket>(io);

//     try
//     {
//         LOG_DEBUG_TAG(client_id) << "OPEN ip_v4 socket";
//         server_socket->open(client_endpoint.protocol().v4());

//         LOG_DEBUG_TAG(client_id) << "SET_OPTION reuse_address(true)";
//         server_socket->set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));

//         LOG_DEBUG_TAG(client_id) << "BIND [" << client_endpoint.address().to_string() << ":" 
//                               << client_endpoint.port() << "]";
//         server_socket->bind(client_endpoint);

//         LOG_DEBUG_TAG(client_id) << "CONNECT TO [" << server_endpoint.address().to_string() << ":" 
//                               << server_endpoint.port() << "]";
//         server_socket->connect(server_endpoint);

//         LOG_DEBUG_TAG(client_id) << "Setting Async Rx Callback";
//         server_socket->async_receive(boost::asio::buffer(rx_buffer), [=](const boost::system::error_code& ec, size_t bytes){this->rx_callback(ec, bytes); });

//         LOG_DEBUG_TAG(client_id) << "Sending PING to Server(" << server_socket->remote_endpoint().port() << ")";
//         server_socket->send(boost::asio::buffer(tx_buffer));

//         LOG_DEBUG_TAG(client_id) << "Starting io_context run";
//         io.run();
//     }
//     catch(const std::exception& e)
//     {
//         LOG_ERROR_TAG(client_id) << e.what();
//         exit (EXIT_FAILURE);
//     }
// }

template<typename Protocol>
void Client<Protocol>::rx_callback(const boost::system::error_code& ec, size_t bytes)
{
    LOG_DEBUG_TAG(client_id) << "Got something!";
    
    if(ec)
    {
        LOG_WARNING_TAG(client_id) << "Erro code: " << ec.message();
        if(ec == boost::asio::error::eof)
        {
            LOG_DEBUG_TAG(client_id) << "Server closed the connection!";
        }
        return;
    }

    LOG_DEBUG_TAG(client_id) << "Received " << bytes << " bytes";
    if(bytes)
    {
        std::string payload(rx_buffer.begin(), rx_buffer.end());
        LOG_DEBUG_TAG(client_id) << "Rx Payload: " << payload;

        if(handler)
        {
            std::unique_ptr<Payload> rxPayload = std::make_unique<Payload>(rx_buffer);
            handler(std::move(rxPayload));
        }
        else
        {
            // if no handler is defined simply Pong the client (use as default impl - maybe be comment out this section later)
            std::this_thread::sleep_for(std::chrono::seconds(2));
            try
            {
                LOG_DEBUG_TAG(client_id) << "Sending PING to Server(" << server_socket->remote_endpoint().port() << ")";
                server_socket->send(boost::asio::buffer(tx_buffer));
            }
            catch(const std::exception& e)
            {
                LOG_ERROR_TAG(client_id) << e.what();
            }            
        }
    }

    LOG_DEBUG_TAG(client_id) << "Setting Async Rx Callback";
    server_socket->async_receive(boost::asio::buffer(rx_buffer), 
        [=](const boost::system::error_code& ec, size_t bytes)
        {
            this->rx_callback(ec, bytes); 
        });
}

template class Client<Tcp>;
template class Client<Udp>;

}
