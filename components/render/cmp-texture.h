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
			std::string path;
            Cmp_Texture(std::string p) : path(p){};
        };
    }
}