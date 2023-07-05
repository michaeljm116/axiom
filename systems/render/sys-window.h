#pragma once
#include "cmp-window.h"
#include <GLFW/glfw3.h>
#include <cstdint>
#include <flecs-world.h>

namespace Axiom {

  namespace Window {
    void init(std::string title, int w, int h);
    void destruct();
    void update(flecs::entity e, Cmp_Window& cmp_wind);
    void resize(flecs::entity e, Cmp_Window& cmp_wind, Cmp_Window_Change& cmp_change);
  };

}  // namespace Axiom
