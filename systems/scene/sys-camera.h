#pragma once

#include "cmp-camera.h"
#include "cmp-input.h"
#include "cmp-transform.h"


namespace Axiom
{
    namespace Scene
    {
        namespace Camera
        {
            void initialize();
            void update_camera(flecs::entity e, Cmp_Transform& t, Cmp_Camera& c);
        }
    }
}


