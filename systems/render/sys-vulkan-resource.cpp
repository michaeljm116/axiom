#include "sys-vulkan-resource.h"
#include "flecs-world.h"

namespace Axiom{
    namespace Render{
        namespace Resources
        {
            Memory::Manager<Texture> g_texture_manager;
            Memory::Manager<Geometry::Model> g_model_manager;
            Memory::Manager<Material::PBR> g_material_manager;
            void initialize()
            {
                g_world.observer<Cmp_Texture>()
                .event(flecs::OnSet)
                .each([](flecs::entity e, Cmp_Texture& t){
                    auto* vulkan = g_world.get_mut<Cmp_Vulkan>();
                    t.index = g_texture_manager.add_resource(Texture(t.path), t.name);
                    auto& texture = g_texture_manager.get_last_added_resource();
                    texture.CreateTexture(vulkan->device);
                });

                g_world.observer<Cmp_Material_PBR>()
                .event(flecs::OnSet)
                .each([](flecs::entity e, Cmp_Material_PBR& m){
                    auto* vulkan = g_world.get_mut<Cmp_Vulkan>();
                    m.index = g_material_manager.add_resource(Material::PBR(
                        m.albedo, m.metallic, m.roughness,
                        m.albedo_texture, m.metallic_texture, m.roughness_texture, m.normal_texture
                    ), m.name);
                    
                    auto& material = g_material_manager.get_last_added_resource();
                    if(g_world.lookup(material.texture_albedo.name.c_str()) == false;
                });             
            }
        }
    }
}