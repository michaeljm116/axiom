#pragma once

#include <flecs.h>
#include "cmp-resource.h"

namespace Axiom{
    namespace Resource{
        bool load_assimp_model(flecs::entity e, Cmp_Resource& res, Cmp_AssimpModel& cmp_mod);
    }
}