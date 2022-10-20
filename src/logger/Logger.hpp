#pragma once

#include <iostream>
#include <sstream>
#include <ostream>
#include <streambuf>
#include <thread>
#include <mutex>
#include <chrono>
#include <iomanip>

namespace network
{

std::string formatFunction(const std::string& function_name, const std::string& class_name = std::string());

enum class LogLevel : uint8_t
{
    NONE = 0,
    FATAL = 1,
    ERROR = 2,
    WARNING = 3,
    INFO = 4,
    DEBUG = 5,
    VERBOSE = 6
};

class Logger : public std::ostream
{
public:
    Logger(LogLevel level);
    ~Logger();

    static void setMaximumLogLevel(LogLevel level);

private:
    class buffer : public std::streambuf {
    public:
        int_type overflow(int_type);
        std::streamsize xsputn(const char *, std::streamsize);

        std::stringstream data_;
    };

    LogLevel level_;
    static LogLevel maxLevel_;
    buffer buffer_;
    std::chrono::system_clock::time_point when_;
    static std::mutex mutex__;

};

#define LOG_FATAL network::Logger(network::LogLevel::FATAL) << formatFunction(__func__) << " "
#define LOG_ERROR network::Logger(network::LogLevel::ERROR) << formatFunction(__func__) << " "
#define LOG_WARNING network::Logger(network::LogLevel::WARNING) << formatFunction(__func__) << " "
#define LOG_INFO network::Logger(network::LogLevel::INFO) << formatFunction(__func__) << " "
#define LOG_DEBUG network::Logger(network::LogLevel::DEBUG) << formatFunction(__func__) << " "
#define LOG_VERBOSE network::Logger(network::LogLevel::VERBOSE) << formatFunction(__func__) << " "

#define LOG_FATAL_TAG(tag) network::Logger(network::LogLevel::FATAL) << formatFunction(__func__, tag) << " "
#define LOG_ERROR_TAG(tag) network::Logger(network::LogLevel::ERROR) << formatFunction(__func__, tag) << " "
#define LOG_WARNING_TAG(tag) network::Logger(network::LogLevel::WARNING) << formatFunction(__func__, tag) << " "
#define LOG_INFO_TAG(tag) network::Logger(network::LogLevel::INFO) << formatFunction(__func__, tag) << " "
#define LOG_DEBUG_TAG(tag) network::Logger(network::LogLevel::DEBUG) << formatFunction(__func__, tag) << " "
#define LOG_VERBOSE_TAG(tag) network::Logger(network::LogLevel::VERBOSE) << formatFunction(__func__, tag) << " "

}
