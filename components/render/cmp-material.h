#pragma once

#include <glm/glm.hpp>
#include "cmp-texture.h"
#include "vulkan/buffer.hpp"
namespace Axiom
{
    namespace Render
    {
        struct Cmp_Material
        {
            int id = 0;
            int unique_id = 0;

            Cmp_Material(){};
            Cmp_Material(int i) : id(i) {};
            Cmp_Material(int i, int ui) : id(i), unique_id(ui){};
        };

        struct Cmp_Material_PBR_Data {
            glm::vec4 albedo = glm::vec4(1.f, 1.f, 1.f, 1.f);
            float metallic = 0.f;
            float roughness = 0.f;

            bool has_albedo = false;
            bool has_metallic = false;
            bool has_roughness = false;
            bool has_normal = false;

            std::string albedo_texture;
            std::string metallic_texture;
            std::string roughness_texture;
            std::string normal_texture;
            
            // Default constructor
            Cmp_Material_PBR_Data() = default;

            // Parameterized constructor
            Cmp_Material_PBR_Data(glm::vec4 a, float m, float r, std::string at, std::string mt, std::string rt, std::string nt)
            : albedo(a), metallic(m), roughness(r), 
            albedo_texture(std::move(at)), metallic_texture(std::move(mt)), 
            roughness_texture(std::move(rt)), normal_texture(std::move(nt)) 
            {
                has_albedo = !albedo_texture.empty();
                has_metallic = !metallic_texture.empty();
                has_roughness = !roughness_texture.empty();
                has_normal = !normal_texture.empty();
            }

            Cmp_Material_PBR_Data(const Cmp_Material_PBR_Data& other) = default;
            Cmp_Material_PBR_Data(Cmp_Material_PBR_Data&& other) noexcept = default;
            Cmp_Material_PBR_Data& operator=(const Cmp_Material_PBR_Data& other) = default;
            Cmp_Material_PBR_Data& operator=(Cmp_Material_PBR_Data&& other) noexcept = default;
        };

        struct Cmp_Material_PBR_Ref {
            uint32_t index = 0;
            std::string name;

            Cmp_Material_PBR_Ref() = default;
            Cmp_Material_PBR_Ref(uint32_t i, std::string n) : index(i), name(std::move(n)){}
            Cmp_Material_PBR_Ref(const Cmp_Material_PBR_Ref& other) = default;
            Cmp_Material_PBR_Ref(Cmp_Material_PBR_Ref&& other) noexcept = default;
            Cmp_Material_PBR_Ref& operator=(const Cmp_Material_PBR_Ref& other) = default;
            Cmp_Material_PBR_Ref& operator=(Cmp_Material_PBR_Ref&& other) noexcept = default;
        };


        
        struct Material_PBR
        {
            std::string name = "";
            struct Uniform {
                glm::vec4 albedo = glm::vec4(1.f, 1.f, 1.f, 1.f);
                float metallic = 0.f;
                float roughness = 0.f;

                bool has_albedo = false;
                bool has_metallic = false;
                bool has_roughness = false;
                bool has_normal = false;

                Uniform() = default;
                Uniform(Cmp_Material_PBR_Data d) : albedo(d.albedo), metallic(d.metallic), roughness(d.roughness),
                    has_albedo(d.has_albedo), has_metallic(d.has_metallic), has_roughness(d.has_roughness), has_normal(d.has_normal) {}
            } uniform;
            
            Vulkan::VBuffer<Uniform> uniform_buffer;

            Cmp_Texture texture_albedo = Cmp_Texture("", 0);
            Cmp_Texture texture_metallic = Cmp_Texture("", 0);
            Cmp_Texture texture_roughness = Cmp_Texture("", 0);
            Cmp_Texture texture_normal = Cmp_Texture("", 0);
            std::vector<VkDescriptorSet> descriptor_sets = {};
            //VkDescriptorSetLayout descriptor_set_layout = {};

            Material_PBR() = default;
            
            Material_PBR(const Material_PBR& other) {
                name = other.name;
                uniform = other.uniform;
                //uniform_buffer = other.uniform_buffer;
                texture_albedo = other.texture_albedo;
                texture_metallic = other.texture_metallic;
                texture_roughness = other.texture_roughness;
                texture_normal = other.texture_normal;
                descriptor_sets = other.descriptor_sets;
                //descriptor_set_layout = other.descriptor_set_layout;
            }
            
            Material_PBR& operator=(const Material_PBR& other) {
                if (this != &other) {
                    name = other.name;
                    uniform = other.uniform;
                    //uniform_buffer = other.uniform_buffer;
                    texture_albedo = other.texture_albedo;
                    texture_metallic = other.texture_metallic;
                    texture_roughness = other.texture_roughness;
                    texture_normal = other.texture_normal;
                    descriptor_sets = other.descriptor_sets;
                    //descriptor_set_layout = other.descriptor_set_layout;
                }
                return *this;
            }
            
            Material_PBR(std::string n, Cmp_Material_PBR_Data d) : name(std::move(n)), uniform(d) {}
        };

        
    }
    
} // namespace Axiom
