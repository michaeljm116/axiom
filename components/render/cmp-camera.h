#pragma once
#include <glm/glm.hpp>

namespace Axiom{
    namespace Render{
        struct Cmp_Camera{
            float aspect_ratio;
            float fov;
            glm::mat4 rot_m;
            Cmp_Camera(){};
            Cmp_Camera(float ar, float f) : aspect_ratio(ar), fov(f){rot_m = glm::mat4();};
        };
    }
}