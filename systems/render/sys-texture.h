#pragma once

#include "vulkan/base.hpp"
#include "cmp-texture.h"
#include "cmp-material.h" 
#include "cmp-resource.h"
#include "flecs-world.h"

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#endif

namespace Axiom
{
    namespace Render
    {
        namespace TextureManager
        {
            void initialize();
            void destroy(Cmp_Texture& t , Cmp_Vulkan& v);
            VkResult create_texture(Cmp_Texture& t, Cmp_Vulkan& v);
            void update_descriptor(Cmp_Texture& t);
        }
    }
}