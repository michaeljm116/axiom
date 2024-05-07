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

namespace Axiom {

    namespace Log {
        void initialize();
        //This Creates/Opens the logfile
        void save(std::string message);
        void send(Axiom::Log::Level level, std::string message);
        bool check(bool b, std::string message);    
    };
}
