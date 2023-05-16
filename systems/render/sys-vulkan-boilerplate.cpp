#include "pch.h"
#include <flecs-world.h>
#include "sys-vulkan-boilerplate.h"
#include "../components/core/cmp-log.h"

namespace axiom
{
    namespace vulkany
    {
        bool Check(bool result, std::string msg){
            g_world.set<Cmp_Log>({log::Level::CHECK, !result, msg});
            return result;
        }

        void Init(){

        }

    }
}