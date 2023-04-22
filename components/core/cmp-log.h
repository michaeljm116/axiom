#pragma once
#include "timer.h"
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <mutex>

namespace axiom{
class Logger {
public:
    enum class Level { DEBUG, INFO, WARNING, ERROR };

    static Logger& GetInstance() {
        static Logger logger_instance;
        return logger_instance;
    }

    void Log(Level level, const std::string& message) {
        std::stringstream ss;
        ss << timer_.current_time() << " - ";

        switch (level) {
            case Level::DEBUG:
                ss << "[DEBUG] ";
                break;
            case Level::INFO:
                ss << "[INFO] ";
                break;
            case Level::WARNING:
                ss << "[WARNING] ";
                break;
            case Level::ERROR:
                ss << "[ERROR] ";
                break;
        }

        ss << message << std::endl;
        std::string log_line = ss.str();
        std::cout << log_line;

        std::unique_lock<std::mutex> lock(log_mutex_);
        if (log_file_.is_open()) {
            log_file_ << log_line;
            log_file_.flush();
        }
        lock.unlock();
    }

    void ErrorCheck(bool condition, const std::string& msg) {
    if (!condition) {
        Log(Level::ERROR, "Failed When " + msg);
    } else {
        Log(Level::INFO, msg);
    }
}


private:
    Logger(const std::string& log_filename = "logs.txt") : log_file_(log_filename, std::ios::app) {
        if (!log_file_.is_open()) {
            std::cerr << "Failed to open log file: " << log_filename << std::endl;
        }
    }

    ~Logger() {
        if (log_file_.is_open()) {
            log_file_.close();
        }
    }

    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    std::ofstream log_file_;
    Timer timer_;
    std::mutex log_mutex_;
};

inline void LogDebug(const std::string& message) {
  Logger::GetInstance().Log(Logger::Level::DEBUG, message);
}

inline void LogInfo(const std::string& message) {
  Logger::GetInstance().Log(Logger::Level::INFO, message);
}

inline void LogWarning(const std::string& message) {
  Logger::GetInstance().Log(Logger::Level::WARNING, message);
}

inline void LogError(const std::string& message) {
  Logger::GetInstance().Log(Logger::Level::ERROR, message);
}

inline void Check(int condition, const std::string& message){
  Logger::GetInstance().ErrorCheck(condition, message);
}

}