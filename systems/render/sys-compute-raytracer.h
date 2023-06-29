#pragma once
#include "../components/render/cmp-compute-raytracer.h"

namespace Axiom{
    namespace Render{
        namespace compute_raytracer{

            void Initialize(Cmp_Compute_Raytracer& raytracer, Cmp_Vulkan& vulkan);

            class Compute_Raytracer{
                Cmp_Compute_Raytracer* raytracer;
                Cmp_Vulkan* vulkan;
                
            };
        }
    }
}
