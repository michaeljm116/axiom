#pragma once

#include <glm/glm.hpp>
#include "cmp-texture.h"
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

        struct Cmp_Material_PBR
        {
            int index = 0;
            std::string name;
            glm::vec4 albedo = glm::vec4(1.f, 1.f, 1.f, 1.f);
            float metallic = 0.f;
            float roughness = 0.f;
            

        };
        struct Cmp_Material_Paths_PBR
        {
            std::string albedo_texture;
            std::string metallic_texture;
            std::string roughness_texture;
            std::string normal_texture;
        };

        
        struct Material_PBR
        {
            std::string name = "";
            glm::vec4 albedo = glm::vec4(1.f, 1.f, 1.f, 1.f);
            float metallic = 0.f;
            float roughness = 0.f;

            Cmp_Texture texture_albedo = Cmp_Texture("", 0);
            Cmp_Texture texture_metallic = Cmp_Texture("", 0);
            Cmp_Texture texture_roughness = Cmp_Texture("", 0);
            Cmp_Texture texture_normal = Cmp_Texture("", 0);
            std::vector<VkDescriptorSet> descriptor_sets = {};
            VkDescriptorSetLayout descriptor_set_layout = {};
            Material_PBR(glm::vec4 a, float m, float r)
            : albedo(a), metallic(m), roughness(r){}
        };
        
    }
    
} // namespace Axiom
