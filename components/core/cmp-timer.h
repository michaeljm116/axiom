// timer.h
#pragma once
#include <chrono>
#include <ctime>
#include <iomanip>
#include <string>
#include <sstream>

namespace axiom {
    struct Cmp_Timer{
        std::chrono::steady_clock::time_point start_time;
        double elapsed_seconds = 0;
    };

    struct Cmp_CurrentTime{
        std::string time_str;
    };

class Timer {
public:
    using Clock = std::chrono::steady_clock;

    Timer() : start_time_(Clock::now()) {}

    std::string current_time() const {
        auto now = std::chrono::system_clock::now();
        auto now_time_t = std::chrono::system_clock::to_time_t(now);
        auto now_tm = std::localtime(&now_time_t);

        std::stringstream ss;
        ss << std::put_time(now_tm, "%Y-%m-%d %H:%M:%S");
        return ss.str();
    }

    double elapsed_seconds() const {
        auto now = Clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::duration<double>>(now - start_time_);
        return duration.count();
    }

    void reset() {
        start_time_ = Clock::now();
    }

private:
    Clock::time_point start_time_;
};

}  // namespace axiom
