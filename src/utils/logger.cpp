#include "utils/logger.hpp"
#include <iostream>
#include <chrono>
#include <iomanip>

void Logger::log(const std::string& message, LogLevel level) {
    auto now = std::chrono::system_clock::now();
    auto now_time = std::chrono::system_clock::to_time_t(now);
    auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;

    std::cerr << "[" << std::put_time(std::localtime(&now_time), "%T") 
              << "." << std::setfill('0') << std::setw(3) << now_ms.count() << "] ";

    switch (level) {
        case LogLevel::DEBUG: std::cerr << "[DEBUG] "; break;
        case LogLevel::INFO: std::cerr << "[INFO] "; break;
        case LogLevel::WARNING: std::cerr << "[WARNING] "; break;
        case LogLevel::ERROR: std::cerr << "[ERROR] "; break;
    }

    std::cerr << message << std::endl;
}

void Logger::logError(const std::exception& e, const std::string& context) {
    log(context + ": " + e.what(), LogLevel::ERROR);
}