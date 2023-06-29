#pragma once
#include "../components/core/cmp-timer.h"

namespace Axiom
{
    namespace Timer 
    {
        void Init();
        //Get the currenet time
        std::string Current();
    }
}