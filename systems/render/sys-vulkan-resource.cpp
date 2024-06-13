#include "sys-vulkan-resource.h"
#include "flecs-world.h"

namespace Axiom{
    namespace Render{
        namespace Resources
        {
            Memory::Manager<Texture> g_texture_manager;
            Memory::Manager<Geometry::Model> g_model_manager;
            Memory::Manager<Material_PBR> g_material_manager;

#pragma region helper lambdas
            const auto remove_file_stem2 = [](std::string file_name){
                int index = file_name.size();
                for(index; file_name[index] != '.'; index--)
                {
                    if(index == 0) return std::string("");
                }
                return file_name.substr(0, index);
            };

            auto create_texture = [](const std::string& file, Cmp_Vulkan** vulkan){
                auto name = remove_file_stem2(file);
                uint32_t index = 0;
                if(!file.empty()){
                    index = g_texture_manager.add_resource(Texture(file), name);
                    auto& texture = g_texture_manager.get_last_added_resource();
                    auto* vk = *vulkan;
                    texture.CreateTexture(vk->device);
                }
                return Cmp_Texture(name, index);
            };
#pragma endregion helper lambdas

            void initialize()
            {
                /*g_world.observer<Cmp_Texture>()
                .event(flecs::OnSet)
                .each([](flecs::entity e, Cmp_Texture& t){
                    auto* vulkan = g_world.get_mut<Cmp_Vulkan>();
                    t.index = g_texture_manager.add_resource(Texture(t.path), t.name);
                    auto& texture = g_texture_manager.get_last_added_resource();
                    texture.CreateTexture(vulkan->device);
                });*/
                g_world.observer<Cmp_AssimpModel, Cmp_Assmbled>()
                .event(flecs::OnSet)
                .each([](flecs::entity e, Cmp_AssimpModel& m, Cmp_Assmbled& a){
                    auto* vulkan = g_world.get_mut<Cmp_Vulkan>();
                    auto index = g_model_manager.add_resource(Geometry::Model(&m), m.name);
                    auto& model = g_model_manager.get_last_added_resource();

                    for(auto& mesh : model.meshes){
                        mesh.vertex_buffer.InitStorageBufferCustomSize(vulkan->device, mesh.verts, mesh.verts.size(), mesh.verts.size());
                        mesh.index_buffer.InitStorageBufferCustomSize(vulkan->device, mesh.indices, mesh.indices.size(), mesh.indices.size());
                    }
                    e.set(Geometry::Cmp_Model_PBR(m.name, index));
                    e.set(Cmp_Renderable());
                });

                g_world.observer<Resource::AxMaterial::PBR>()
                .event(flecs::OnSet)
                .each([](flecs::entity e, Resource::AxMaterial::PBR& m)
                {
                    
                    e.set(Cmp_Material_PBR(m.index, m.name, m.albedo.val, m.metalness.val, m.roughness.val));
                    e.set(Cmp_Material_Paths_PBR(m.albedo.file, m.metalness.file, m.roughness.file, m.normal.file));
                });

                g_world.observer<Cmp_Material_PBR,Cmp_Material_Paths_PBR>()
                .event(flecs::OnSet)
                .each([](flecs::entity e, Cmp_Material_PBR& m, Cmp_Material_Paths_PBR& mp){
                    // Make sure the material doesn't already exist
                    // if(g_world.lookup(material.name.c_str()) == false;
                    
                    // Get the vulkan component
                    auto* vulkan = g_world.get_mut<Cmp_Vulkan>();
                    
                    // Make the Material
                    m.index = g_material_manager.add_resource(Material_PBR(m.albedo, m.metallic, m.roughness), m.name);
                    auto& material = g_material_manager.get_last_added_resource();
                    material.name = m.name;

                    // If the material has textures, create the textures
                    material.texture_albedo = create_texture(mp.albedo_texture, &vulkan);
                    material.texture_normal = create_texture(mp.normal_texture, &vulkan);
                    material.texture_metallic = create_texture(mp.metallic_texture, &vulkan);
                    material.texture_roughness = create_texture(mp.roughness_texture, &vulkan);
                    
                    // Once textures are created, create the descriptor set of material
                    e.set(Cmp_Renderable());                    
                });             


                
            }
        }
    }
}