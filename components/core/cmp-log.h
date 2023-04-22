#pragma once
#include <fstream>
#include <string>
#include <mutex>

namespace axiom
{

    struct LogData {
        std::ofstream log_file;
        std::mutex log_mutex;
    };

    struct Cmp_LogFile {
        std::string log_filename = "logs.txt";
        std::shared_ptr<LogData> log_data = std::make_shared<LogData>();
        Cmp_LogFile(std::string fn) : log_filename(fn){};
        Cmp_LogFile(){};
    };

    enum class LogLevel { 
        DEBUG, INFO, WARNING, ERROR, CHECK
    };

    struct Cmp_Log{
        LogLevel lvl;
        bool check = false;
        std::string message;
    };

}