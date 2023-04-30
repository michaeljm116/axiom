#pragma once
#include "../components/core/cmp-timer.h"

namespace axiom
{
    namespace timer 
    {
        void Init();
        //Get the currenet time
        std::string Current();
    }
}