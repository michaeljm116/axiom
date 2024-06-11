#include "sys-vulkan-resource.h"
#include "flecs-world.h"

namespace Axiom{
    namespace Render{
        namespace Resources
        {
            Memory::Manager<Texture> g_texture_manger;
            Memory::Manager<Geometry::Model> g_model_manager;
            Memory::Manager<Material::PBR> g_material_manager;
            void initialize()
            {
                g_world.observer<Cmp_Texture>()
                .event(flecs::OnSet)
                .each([](flecs::entity e, Cmp_Texture& t){
                    auto* vulkan = g_world.get_mut<Cmp_Vulkan>();
                    Texture* texture = new Texture(t.path);
                    texture->CreateTexture(vulkan->device);
                    g_texture_manger.addResource(*texture, t.name);

                });
            }
        }
    }
}