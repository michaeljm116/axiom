/**
 * @file cmp-log.h
 * @author Mike Murrell (mikestoleyobike@aim.com)
 * @brief Log Component
 * @version 0.1
 * @date 2023-05-11
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#pragma once
#include <fstream>
#include <string>
#include <mutex>
#include <memory>

namespace axiom
{
    namespace log{
        struct Data {
            std::ofstream log_file;
            std::mutex log_mutex;
        };

        enum class Level { 
            DEBUG, INFO, WARNING, ERROR, CHECK
        };

    };

    /**
     * @brief LogFile Component
     * @param log_filename @param log_data 
     */
    struct Cmp_LogFile {
        std::string log_filename = "../../doc/logs.txt";
        std::shared_ptr<log::Data> log_data = std::make_shared<log::Data>();
        Cmp_LogFile(std::string fn) : log_filename(fn){};
        Cmp_LogFile(){};
    };

    /**
    * @brief Log Component
    * @param lvl Level of the log (Info/debug/error/warning/check)
    * @param check if true, it calls the check function
    * @param message the log's message
    */
    struct Cmp_Log{
        log::Level lvl;
        bool check = false;
        std::string message;
    };



}