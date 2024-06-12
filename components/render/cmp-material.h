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
            uint32_t index = 0;
            std::string name;
            glm::vec4 albedo = glm::vec4(1.f, 1.f, 1.f, 1.f);
            float metallic = 0.f;
            float roughness = 0.f;
            Cmp_Material_PBR(uint32_t i, std::string n, glm::vec4 a, float m, float r)
            : index(i), name(n), albedo(a), metallic(m), roughness(r)
            {}
        };
        struct Cmp_Material_Paths_PBR
        {
            std::string albedo_texture;
            std::string metallic_texture;
            std::string roughness_texture;
            std::string normal_texture;
            Cmp_Material_Paths_PBR(std::string a, std::string m, std::string r, std::string n)
             : albedo_texture(a), metallic_texture(m), roughness_texture(r), normal_texture(n)
            {}
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
