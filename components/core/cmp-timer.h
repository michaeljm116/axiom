/**
 * @file cmp-timer.h
 * @author mike murrell (mikestoleyobike@aim.com)
 * @brief Timer Component
 * @version 0.1
 * @date 2023-05-09
 * 
 * @copyright Copyright (c) 2023
 * */
#pragma once
#include <chrono>
#include <string>

namespace Axiom {
    /**
     * @brief Timer Component, add this to start recording time, remove it to end
     * @param start_time The start time
     * @param elapsed_seconds The Time Elapsed
     */
    struct Cmp_Timer{
        std::chrono::steady_clock::time_point start_time;
        double elapsed_seconds = 0;
    };

    /**
    * @brief Current Time Component, Set this when you want the current time
    * @param time_str The Time in String format
    */
    struct Cmp_CurrentTime{
        std::string time_str;
    };

}  // namespace Axiom
