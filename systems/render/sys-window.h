#pragma once
#include "cmp_window.h"
#include <GLFW/glfw3.h>
#include <cstdint>
#include <flecs.h>

namespace axiom {

  class Sys_Window {
  public:

    Sys_Window(flecs::world & world_, std::string title int w, int h);
    ~Sys_Window();
    void Init(flecs::entity e, Cmp_Window cmp_wind);
    void Update(flecs::entity e, Cmp_Window cmp_wind);
    void Resize(flecs::entity e, Cmp_Window cmp_wind, Cmp_Window_Change cmp_change);
    
    private:
    flecs::world* world;
  };

}  // namespace axiom
