#pragma once

#include "../common/types.h"
#include <fstream>
#include <sstream>
#include <chrono>
#include <iomanip>

namespace XShade {
    enum class LogLevel {
        Debug,
        Info,
        Warning,
        Error
    };

    class Logger {
    public:
        static void Initialize(const std::string& logFile = "xshade.log");
        static void Shutdown();
        
        static void Debug(const std::string& message);
        static void Info(const std::string& message);
        static void Warning(const std::string& message);
        static void Error(const std::string& message);
        
        static void SetLogLevel(LogLevel level);
        static void SetConsoleOutput(bool enable);

    private:
        static void Log(LogLevel level, const std::string& message);
        static std::string GetTimestamp();
        static std::string LogLevelToString(LogLevel level);

        static std::unique_ptr<std::ofstream> log_file_;
        static std::mutex log_mutex_;
        static LogLevel min_log_level_;
        static bool console_output_;
        static bool initialized_;
    };
}
