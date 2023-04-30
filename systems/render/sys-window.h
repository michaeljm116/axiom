#pragma once
#include "cmp-window.h"
#include <GLFW/glfw3.h>
#include <cstdint>
#include <flecs-world.h>

namespace axiom {

  namespace window {
    void Init(std::string title, int w, int h);
    void Destruct();
    void Update(flecs::entity e, Cmp_Window& cmp_wind);
    void Resize(flecs::entity e, Cmp_Window& cmp_wind, Cmp_Window_Change& cmp_change);
  };

}  // namespace axiom
