#pragma once

#include "memory-manager.h"
#include "texture.h"
#include "cmp-material.h"
#include "cmp-geometry.h"
#include "cmp-vulkan.h"

namespace Axiom{
    namespace Render
    {
        namespace Resources{
            extern Memory::Manager<Texture> g_texture_manager;
            extern Memory::Manager<Geometry::Model> g_model_manager;
            extern Memory::Manager<Material::PBRMaterial> g_material_manager;

            void initialize();
        }
    }
}