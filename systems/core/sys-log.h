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

    namespace log {
        void Init();
        //This Creates/Opens the logfile
        void Save(std::string message);
        void Set(axiom::LogLevel level, std::string message);
        void Check(bool b, std::string message);    
    };
}
