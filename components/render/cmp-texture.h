#pragma once
#include "vulkan/base.hpp"
#include "cmp-vulkan.h"

namespace Axiom
{
    namespace Render
    {
        struct Cmp_Texture
        {
            std::string name;
			uint32_t index;
            bool exists = false;
            Cmp_Texture(std::string n, uint32_t i) : name(n), index(i){
                if(!(name == "")) exists = true;
            };
        };
    }
}