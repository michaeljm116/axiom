#pragma once
#include "cmp-timer.h"
#include <flecs.h>

namespace axiom{
    class Sys_Timer {
    public:
        Sys_Timer(flecs::world& world) {
            world.system<Cmp_Timer>("TimerOnAddSystem")
                .kind(flecs::OnAdd)
                .each(&Sys_Timer::on_timer_add);

            world.system<Cmp_Timer>("TimerOnRemoveSystem")
                .kind(flecs::OnRemove)
                .each(&Sys_Timer::on_timer_remove);

            world.singleton<Cmp_CurrentTime>();
        }

        void on_timer_add(flecs::entity e, Cmp_Timer& t) {
            t.start_time = std::chrono::steady_clock::now();
        }

        void on_timer_remove(flecs::entity e, Cmp_Timer& t) {
            auto now = std::chrono::steady_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::duration<double>>(now - t.start_time);
            t.elapsed_seconds = duration.count();

            // Do something with the elapsed_seconds
            // For example, print it to the console
            std::cout << "Elapsed seconds: " << t.elapsed_seconds << std::endl;
        }

        std::string get_current_time(flecs::world& world) {
            auto e = world.singleton<axiom::Cmp_CurrentTime>();
            auto* currentTimeComponent = e.get_mut<Cmp_CurrentTime>();
            currentTimeComponent->time_str = current_time();
            return currentTimeComponent->time_str;
        }

    private:
        std::string current_time() const {
            auto now = std::chrono::system_clock::now();
            auto now_time_t = std::chrono::system_clock::to_time_t(now);
            auto now_tm = std::localtime(&now_time_t);

            std::stringstream ss;
            ss << std::put_time(now_tm, "%Y-%m-%d %H:%M:%S");
            return ss.str();
        }
    };

    inline std::string get_current_time(flecs::world& world){
        Sys_Timer timer(world);
        std::string currentTime = timer.get_current_time(world);
        return currentTime;
    }
}