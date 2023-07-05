#pragma once
#include "../components/core/cmp-timer.h"

namespace Axiom
{
    namespace Timer 
    {
        void initialize();
        //Get the currenet time
        std::string get_current_time();
    }
}