#pragma once
#include "../components/render/cmp-compute-raytracer.h"

namespace Axiom{
    namespace Render{
        namespace Raytracing{

            void initialize(Cmp_Compute_Raytracer& raytracer, Cmp_Vulkan& vulkan);

            class Compute_Raytracer{
                Cmp_Compute_Raytracer* raytracer;
                Cmp_Vulkan* vulkan;
                
            };
        }
    }
}
