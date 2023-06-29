#include "pch.h"
#include "sys-timer.h"
#include <flecs-world.h>
#include <sstream>
#include <ctime>
#include <chrono>
#include <iomanip>

namespace Axiom{
    namespace Timer{
        void Init(){
            // Add Timer
            g_world.observer<Cmp_Timer>("TimerOnAddSystem")
                .event(flecs::OnAdd)
                .each([](flecs::entity e, Cmp_Timer& t){
                    t.start_time = std::chrono::steady_clock::now();
            });

            // Remove Timer
            g_world.observer<Cmp_Timer>("TimerOnRemoveSystem")
                .event(flecs::OnRemove)
                .each([](flecs::entity e, Cmp_Timer& t){
                    auto now = std::chrono::steady_clock::now();
                    auto duration = std::chrono::duration_cast<std::chrono::duration<double>>(now - t.start_time);
                    t.elapsed_seconds = duration.count();
                    std::cout << "Elapsed seconds: " << t.elapsed_seconds << " for: " << e.name() << "\n";
                });
        }

        // Return the Current Time
        std::string Current(){
            auto now = std::chrono::system_clock::now();
            auto now_time_t = std::chrono::system_clock::to_time_t(now);
            std::tm now_tm;

        #if defined(_WIN32) || defined(_WIN64)
            localtime_s(&now_tm, &now_time_t);
        #else
            localtime_r(&now_time_t, &now_tm);
        #endif

            std::stringstream ss;
            ss << std::put_time(&now_tm, "%Y-%m-%d %H:%M:%S");
            return ss.str();
        }
    }
}