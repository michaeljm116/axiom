#pragma once
#include <glm/glm.hpp>

namespace Axiom{
    namespace Render{
        struct Cmp_Light{
            glm::vec3 color;
            float intensity;
            int id = 0;
            Cmp_Light(){};
            Cmp_Light(glm::vec3 c, float i, int id) : color(c), intensity(i), id(id) {};
        };
    }
}