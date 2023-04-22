// timer.h
#pragma once
#include <chrono>
#include <string>

namespace axiom {
    struct Cmp_Timer{
        std::chrono::steady_clock::time_point start_time;
        double elapsed_seconds = 0;
    };

    struct Cmp_CurrentTime{
        std::string time_str;
    };

}  // namespace axiom
