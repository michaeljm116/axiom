// logger.h
#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <mutex>
#include <flecs-world.h>

#include "../components/core/cmp-timer.h"
#include "../components/core/cmp-log.h"

namespace axiom {

    class Sys_Logger {
    public:
        Sys_Logger();
        ~Sys_Logger();

        //This Creates/Opens the logfile
        void OpenLogFile(flecs::entity e, Cmp_LogFile& cmp);
        void CloseLogFile(flecs::entity e, Cmp_LogFile& f);
        void SaveLog(std::string message);
        void OnLog(flecs::entity e, Cmp_Log& l);
        void LogMessage(axiom::LogLevel level, std::string message);
        void CheckMessage(bool b, std::string message);

    };

    inline void Check( bool b, std::string msg){
        g_world.set<axiom::Cmp_Log>({axiom::LogLevel::CHECK, b, msg});
    }
    inline void Log(axiom::LogLevel lvl, std::string msg){
        g_world.set<axiom::Cmp_Log>({lvl, true, msg});
        
    }
    
}
