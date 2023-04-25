// logger.h
#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <mutex>
#include <flecs.h>

#include "../components/core/cmp-timer.h"
#include "../components/core/cmp-log.h"

namespace axiom {

    class Sys_Logger {
    public:
        Sys_Logger(flecs::world& world_);
        ~Sys_Logger();

        //This Creates/Opens the logfile
        void OpenLogFile(flecs::entity e, Cmp_LogFile& cmp);
        void CloseLogFile(flecs::entity e, Cmp_LogFile& f);
        void SaveLog(std::string message);
        void OnLog(flecs::entity e, Cmp_Log& l);
        void LogMessage(axiom::LogLevel level, std::string message);
        void CheckMessage(bool b, std::string message);
        
    private:
        flecs::world* world;
    };

    inline void Check(flecs::world& w, bool b, std::string msg){
        w.set<axiom::Cmp_Log>({axiom::LogLevel::CHECK, b, msg});
    }
    inline void Log(flecs::world& w, axiom::LogLevel lvl, std::string msg){
        w.set<axiom::Cmp_Log>({lvl, true, msg});
    }
    
}
