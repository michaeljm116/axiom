#pragma once
#include <fstream>
#include <string>
#include <mutex>

namespace axiom
{
    struct Cmp_LogFile{
        std::ofstream log_file;
        std::mutex log_mutex;
        std::string log_filename;
        Cmp_LogFile(std::string fn) : log_filename(fn){};
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