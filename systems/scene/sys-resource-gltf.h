#pragma once

#include <flecs.h>
#include "cmp-resource.h"

namespace Axiom{
    namespace Resource{
        bool load_gltfmodel(flecs::entity e, Cmp_Resource& res, Cmp_ResModel& cmp_mod);
    }
}