#pragma once

#include <string>
#include <exception>

class Logger {
public:
    enum class LogLevel {
        DEBUG,
        INFO,
        WARNING,
        ERROR
    };

    static void log(const std::string& message, LogLevel level = LogLevel::INFO);
    static void logError(const std::exception& e, const std::string& context = "");
};