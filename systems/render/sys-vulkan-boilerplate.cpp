#include "pch.h"
#include <flecs-world.h>
#include "sys-vulkan-boilerplate.h"
#include "../components/core/cmp-log.h"

namespace Axiom
{
    namespace Vulkany
    {
        bool Check(bool result, std::string msg){
            g_world.set<Cmp_Log>({Log::Level::CHECK, !result, msg});
            return result;
        }

        void Init(){

        }

    }
}