#pragma once

/**
 * @file Logger.hpp
 * @brief Simple header-only structured logger for SisterSTRATA.
 *
 * Replaces ad-hoc std::cout/std::cerr usage with leveled, prefixed output.
 * Design goals:
 *   - Zero external dependencies
 *   - Thread-safe via std::mutex
 *   - Compile-time minimum level via SISTERSTRATA_LOG_LEVEL
 *   - Drop-in macros: LOG_DEBUG, LOG_INFO, LOG_WARN, LOG_ERROR
 *
 * Usage:
 *   #include "infrastructure/logging/Logger.hpp"
 *   LOG_INFO("NarrativeAnalyzer", "Built graph with {} nodes", nodeCount);
 *
 * To set minimum log level at compile time, define SISTERSTRATA_LOG_LEVEL:
 *   -DSISTERSTRATA_LOG_LEVEL=2  (shows WARN and ERROR only)
 */

#include <iostream>
#include <sstream>
#include <string>
#include <mutex>
#include <chrono>
#include <ctime>
#include <iomanip>

namespace Infrastructure::Logging {

enum class LogLevel : int {
    Debug = 0,
    Info  = 1,
    Warn  = 2,
    Error = 3
};

inline const char* levelTag(LogLevel level) {
    switch (level) {
        case LogLevel::Debug: return "DEBUG";
        case LogLevel::Info:  return "INFO ";
        case LogLevel::Warn:  return "WARN ";
        case LogLevel::Error: return "ERROR";
    }
    return "?????";
}

inline std::string timestamp() {
    auto now = std::chrono::system_clock::now();
    std::time_t tt = std::chrono::system_clock::to_time_t(now);
    std::tm tm{};
#if defined(_WIN32)
    localtime_s(&tm, &tt);
#else
    localtime_r(&tt, &tm);
#endif
    std::ostringstream out;
    out << std::put_time(&tm, "%H:%M:%S");
    return out.str();
}

inline void log(LogLevel level, const std::string& component, const std::string& message) {
    static std::mutex mtx;
    std::lock_guard<std::mutex> lock(mtx);

    auto& stream = (level >= LogLevel::Warn) ? std::cerr : std::cout;
    stream << "[" << timestamp() << "] "
           << "[" << levelTag(level) << "] "
           << "[" << component << "] "
           << message
           << std::endl;
}

} // namespace Infrastructure::Logging

// Compile-time log level filter (default: show everything)
#ifndef SISTERSTRATA_LOG_LEVEL
#define SISTERSTRATA_LOG_LEVEL 0
#endif

#define SS_LOG(level, component, message) \
    do { \
        if (static_cast<int>(level) >= SISTERSTRATA_LOG_LEVEL) { \
            Infrastructure::Logging::log(level, component, message); \
        } \
    } while (0)

#define LOG_DEBUG(component, message) SS_LOG(Infrastructure::Logging::LogLevel::Debug, component, message)
#define LOG_INFO(component, message)  SS_LOG(Infrastructure::Logging::LogLevel::Info,  component, message)
#define LOG_WARN(component, message)  SS_LOG(Infrastructure::Logging::LogLevel::Warn,  component, message)
#define LOG_ERROR(component, message) SS_LOG(Infrastructure::Logging::LogLevel::Error, component, message)
