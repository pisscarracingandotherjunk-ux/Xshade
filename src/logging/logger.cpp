#include "../include/logging/logger.h"
#include <iostream>
#include <windows.h>

namespace XShade {
    std::unique_ptr<std::ofstream> Logger::log_file_;
    std::mutex Logger::log_mutex_;
    LogLevel Logger::min_log_level_ = LogLevel::Info;
    bool Logger::console_output_ = true;
    bool Logger::initialized_ = false;

    void Logger::Initialize(const std::string& logFile) {
        std::lock_guard<std::mutex> lock(log_mutex_);
        
        if (initialized_) {
            return;
        }

        log_file_ = std::make_unique<std::ofstream>(logFile, std::ios::out | std::ios::app);
        if (!log_file_->is_open()) {
            std::cerr << "Failed to open log file: " << logFile << std::endl;
            return;
        }

        initialized_ = true;
        Info("Logger initialized - " + std::string(APPLICATION_NAME) + " v" + VERSION);
    }

    void Logger::Shutdown() {
        std::lock_guard<std::mutex> lock(log_mutex_);
        
        if (initialized_ && log_file_) {
            Info("Logger shutting down");
            log_file_->close();
            log_file_.reset();
        }
        
        initialized_ = false;
    }

    void Logger::Debug(const std::string& message) {
        Log(LogLevel::Debug, message);
    }

    void Logger::Info(const std::string& message) {
        Log(LogLevel::Info, message);
    }

    void Logger::Warning(const std::string& message) {
        Log(LogLevel::Warning, message);
    }

    void Logger::Error(const std::string& message) {
        Log(LogLevel::Error, message);
    }

    void Logger::SetLogLevel(LogLevel level) {
        std::lock_guard<std::mutex> lock(log_mutex_);
        min_log_level_ = level;
    }

    void Logger::SetConsoleOutput(bool enable) {
        std::lock_guard<std::mutex> lock(log_mutex_);
        console_output_ = enable;
    }

    void Logger::Log(LogLevel level, const std::string& message) {
        if (level < min_log_level_) {
            return;
        }

        std::lock_guard<std::mutex> lock(log_mutex_);
        
        std::string timestamp = GetTimestamp();
        std::string level_str = LogLevelToString(level);
        std::string formatted_message = "[" + timestamp + "] [" + level_str + "] " + message;

        // Write to file
        if (initialized_ && log_file_ && log_file_->is_open()) {
            *log_file_ << formatted_message << std::endl;
            log_file_->flush();
        }

        // Write to console
        if (console_output_) {
            if (level >= LogLevel::Error) {
                std::cerr << formatted_message << std::endl;
            } else {
                std::cout << formatted_message << std::endl;
            }
        }

        // Write to OutputDebugString on Windows
#ifdef _WIN32
        OutputDebugStringA((formatted_message + "\n").c_str());
#endif
    }

    std::string Logger::GetTimestamp() {
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()) % 1000;

        std::stringstream ss;
        ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
        ss << '.' << std::setfill('0') << std::setw(3) << ms.count();
        
        return ss.str();
    }

    std::string Logger::LogLevelToString(LogLevel level) {
        switch (level) {
            case LogLevel::Debug:   return "DEBUG";
            case LogLevel::Info:    return "INFO ";
            case LogLevel::Warning: return "WARN ";
            case LogLevel::Error:   return "ERROR";
            default:                return "UNKN ";
        }
    }
}
