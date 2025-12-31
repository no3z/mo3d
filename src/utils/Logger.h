#pragma once

#include <iostream>
#include <string>
#include <sstream>
#include <chrono>
#include <iomanip>

namespace mo3d {

enum class LogLevel {
    Debug,
    Info,
    Warning,
    Error
};

class Logger {
public:
    static Logger& Instance() {
        static Logger instance;
        return instance;
    }

    template<typename... Args>
    void Debug(Args&&... args) {
        Log(LogLevel::Debug, std::forward<Args>(args)...);
    }

    template<typename... Args>
    void Info(Args&&... args) {
        Log(LogLevel::Info, std::forward<Args>(args)...);
    }

    template<typename... Args>
    void Warning(Args&&... args) {
        Log(LogLevel::Warning, std::forward<Args>(args)...);
    }

    template<typename... Args>
    void Error(Args&&... args) {
        Log(LogLevel::Error, std::forward<Args>(args)...);
    }

    void SetLevel(LogLevel level) { minLevel = level; }

private:
    Logger() : minLevel(LogLevel::Info) {}
    LogLevel minLevel;

    template<typename... Args>
    void Log(LogLevel level, Args&&... args) {
        if (level < minLevel) return;

        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);

        std::ostringstream oss;
        oss << "[" << std::put_time(std::localtime(&time), "%H:%M:%S") << "] ";

        switch (level) {
            case LogLevel::Debug:   oss << "[DEBUG] "; break;
            case LogLevel::Info:    oss << "[INFO] "; break;
            case LogLevel::Warning: oss << "[WARN] "; break;
            case LogLevel::Error:   oss << "[ERROR] "; break;
        }

        (oss << ... << args);
        std::cout << oss.str() << std::endl;
    }
};

#define LOG_DEBUG(...) mo3d::Logger::Instance().Debug(__VA_ARGS__)
#define LOG_INFO(...) mo3d::Logger::Instance().Info(__VA_ARGS__)
#define LOG_WARN(...) mo3d::Logger::Instance().Warning(__VA_ARGS__)
#define LOG_ERROR(...) mo3d::Logger::Instance().Error(__VA_ARGS__)

} // namespace mo3d
