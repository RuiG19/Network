#include <iostream>
#include <thread>
#include <future>
#include <boost/asio.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

#include "Logger.hpp"
#include "TcpServer.hpp"
#include "TcpClient.hpp"

#include <map>

using namespace network;

typedef struct
{
    std::string ip;
    uint16_t server_port;
    uint16_t client_port;
    uint16_t clients_number;
    network::LogLevel logLevel;
} EnvConfig;

static EnvConfig configurations = 
{
    // Default values if config file is not present
    "127.0.0.1",
    31490,
    31400,
    1,
    network::LogLevel::DEBUG
};

static const std::map<std::string, network::LogLevel> logLevelMap = 
{
    {"ERROR", network::LogLevel::DEBUG},
    {"WARNING", network::LogLevel::WARNING},
    {"DEBUG", network::LogLevel::DEBUG}
};

static const std::string configFileName("cfg/NetworkTest_config.json");

static void loadConfigurations()
{
    LOG_DEBUG << "Loading config file: " << configFileName.c_str();
    boost::property_tree::ptree root;

    std::string logLevel = "DEBUG";

    try
    {
        boost::property_tree::read_json(configFileName.c_str(), root);
        configurations.ip = root.get<std::string>("ip");
        configurations.server_port = root.get<uint16_t>("server_port");
        configurations.client_port = root.get<uint16_t>("client_port");
        configurations.clients_number = root.get<uint16_t>("clients_number");
        logLevel = root.get<std::string>("log_level");
        configurations.logLevel = logLevelMap.at(logLevel);

    }
    catch(const std::exception& e)
    {
        LOG_ERROR << "Failed to load config file(" << configFileName.c_str() << "). Will use default configs.";
        LOG_ERROR << e.what();
    }

    LOG_DEBUG << "Ip: " << configurations.ip.c_str() 
                       << ", server_port: " << configurations.server_port 
                       << ", client_port: " << configurations.client_port
                       << ", clients_number: " << configurations.clients_number
                       << ", logLevel: " << logLevel;

}

enum class TestMode : uint8_t
{
    All,
    Server,
    Client
};

static const std::map<std::string, TestMode> testModeMap = 
{
    {"-s", TestMode::Server},
    {"-c", TestMode::Client}
};

static TestMode testMode = TestMode::All;

static void setTestMode(int argc, char *argv[])
{
    
    if(argc > 1)
    {
        std::string _argv = std::string(argv[1]);
        LOG_DEBUG << "argv[1] = " << _argv;

        if(testModeMap.find(_argv) != testModeMap.end())
        {
            testMode = testModeMap.at(_argv);
            LOG_DEBUG << ((testMode == TestMode::Server) ?
                         "Setting test mode to run only the Server" 
                         : "Setting test mode to run only the Clients");
        }
        else
        {
            LOG_WARNING << "Invalid argument. Will ignore and run both Server and Clients!";
        }
    }
}

// ############# MAIN #############


int main(int argc, char *argv[])
{    
    LOG_DEBUG << "MAIN start";

    setTestMode(argc, argv);
    loadConfigurations();
    std::string ip = configurations.ip;
    uint16_t server_port = configurations.server_port;
    uint16_t client_port = configurations.client_port;
    uint16_t numberOfClients = configurations.clients_number;
    Logger::setMaximumLogLevel(configurations.logLevel);

    TcpServer server(ip, server_port);
    if(testMode != TestMode::Client)
    {
        LOG_DEBUG << "Launching server thread";
        server.start();
    }

    std::vector<std::unique_ptr<TcpClient>> clients;
    clients.clear();
    if(testMode != TestMode::Server)
    {
        for(uint16_t i = 0; i < numberOfClients; ++i)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(100)); // some time so the server and the previouse client can init
            
            clients.emplace_back(std::make_unique<TcpClient>(ip, client_port + i, ip, server_port));
            LOG_DEBUG << "Launching Client " << (uint16_t)(clients.at(i)->getId()) << " thread";
            clients.at(i)->start();
        }

        LOG_DEBUG << "Clients started: " << clients.size();
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(100)); // some time so the server and the previouse client can init
    
    while(true)
    {
        LOG_DEBUG << "Idle on main loop. CTRL + C to stop";
        bool server_status = false;
        std::stringstream statusLog;

        if(testMode != TestMode::Client)
        {
            server_status = server.status() != std::future_status::ready;
            statusLog << "Child threads status: server(" 
                    << (server_status ? "\033[1;32m RUNNING \033[0m" : "\033[1;31m STOPPED \033[0m");
        }

        if(testMode != TestMode::Server)
        {
            LOG_DEBUG << "Clients running: " << clients.size();
            
            for(uint16_t i = 0; i < clients.size(); )
            {
                bool current_client_status = clients.at(i)->status() != std::future_status::ready;

                if(testMode == TestMode::All)
                {
                    statusLog << "), client_" << (uint16_t)clients.at(i)->getId() << "(" 
                              << (current_client_status ? "\033[1;32m RUNNING \033[0m" : "\033[1;31m STOPPED \033[0m");
                }
                else
                {
                    if(i == 0)
                    {
                        statusLog << "Child threads status: client_";
                    }
                    else
                    {
                        statusLog << "), client_";
                    }
                    statusLog << (uint16_t)clients.at(i)->getId() << "(" << (current_client_status ? "\033[1;32m RUNNING \033[0m" : "\033[1;31m STOPPED \033[0m");
                }


                if(!current_client_status)
                {
                    clients.erase(clients.begin() + i);
                }
                else
                {
                    ++i;
                }
            }
        }
        statusLog << ")";
        LOG_DEBUG << statusLog.str();

        if(!server_status && ( clients.size() == 0))
        {
            LOG_DEBUG << "All threads are STOPPED. Will exit main loop.";
            break;
        }

        std::this_thread::sleep_for(std::chrono::seconds(2));
    }

    LOG_DEBUG << "MAIN end";

    return 0;
}
