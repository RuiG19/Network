#include "Logger.hpp"


namespace network
{

//          foreground background
// black        30         40
// red          31         41
// green        32         42
// yellow       33         43
// blue         34         44
// magenta      35         45
// cyan         36         46
// white        37         47

// reset             0  (everything back to normal)
// bold/bright       1  (often a brighter shade of the same colour)
// underline         4
// inverse           7  (swap foreground and background colours)
// bold/bright off  21
// underline off    24
// inverse off      27

// example \033[1;31m -> 1 for bold  31 for red text

#define COLOR_RESET "\033[0m"
#define COLOR_CONTEXT "\033[1;31m" 
#define COLOR_GENERAL ""
#define COLOR_WARNING "\033[21;33m"
#define COLOR_ERROR "\033[1;101m"


std::string formatFunction(const std::string& function_name, const std::string& class_name)
{
    std::stringstream log_id;
    uint16_t thread_id = (uint16_t)std::hash<std::thread::id>{}(std::this_thread::get_id());

    std::string classIdColumn = (class_name.size() > 0) ? "::" : std::string();

    // log_id << COLOR_CONTEXT << class_name << classIdColumn << function_name << "(" << (int)thread_id << ")" << COLOR_RESET;
    log_id << COLOR_CONTEXT << "(" << (int)thread_id << ") " << class_name << classIdColumn << function_name << COLOR_RESET;
    // log_id << class_name << classIdColumn << function_name << "(" << (int)thread_id << "): ";

    return log_id.str();
}

std::mutex Logger::mutex__;

Logger::Logger(LogLevel level) : level_(level), std::ostream(&buffer_), when_(std::chrono::system_clock::now())
{

}

Logger::~Logger()
{
    std::lock_guard<std::mutex> its_lock(mutex__);

    if(level_ > maxLevel_) return;

    // time stamp
    auto its_time_t = std::chrono::system_clock::to_time_t(when_);
    auto its_time = std::localtime(&its_time_t);
    auto its_ms = (when_.time_since_epoch().count() / 100) % 1000000;

    std::string msg_color;
    std::string msg_logLevel;

    switch (level_)
    {
    case LogLevel::FATAL:
    case LogLevel::ERROR:
        msg_color = COLOR_ERROR;
        msg_logLevel = "[ERROR]";
        break;
    case LogLevel::WARNING:
        msg_color = COLOR_WARNING;
        msg_logLevel = "[WARNING]";
        break;
    case LogLevel::INFO:
    case LogLevel::DEBUG:
    case LogLevel::VERBOSE:
    default:
        msg_color = COLOR_GENERAL;
        msg_logLevel = "[DEBUG]";
        break;
    }

    std::stringstream log_msg;
    log_msg << msg_color
            << std::dec << std::setw(4) << its_time->tm_year + 1900 << "-"
            << std::dec << std::setw(2) << std::setfill('0') << its_time->tm_mon << "-"
            << std::dec << std::setw(2) << std::setfill('0') << its_time->tm_mday << " "
            << std::dec << std::setw(2) << std::setfill('0') << its_time->tm_hour << ":"
            << std::dec << std::setw(2) << std::setfill('0') << its_time->tm_min << ":"
            << std::dec << std::setw(2) << std::setfill('0') << its_time->tm_sec << "."
            << std::dec << std::setw(6) << std::setfill('0') << its_ms 
            << " " << msg_logLevel << " "
            << COLOR_RESET
            << buffer_.data_.str();

    // TODO: Add alternatives to cout like DLT
    std::cout << log_msg.str() << std::endl;
}

LogLevel Logger::maxLevel_ = network::LogLevel::DEBUG;

void Logger::setMaximumLogLevel(LogLevel level)
{
    maxLevel_ = level;
}


std::streambuf::int_type Logger::buffer::overflow(std::streambuf::int_type c) {
    if (c != EOF) {
        data_ << (char)c;
    }

    return (c);
}

std::streamsize Logger::buffer::xsputn(const char *s, std::streamsize n) {
    data_.write(s, n);
    return (n);
}

}