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

        struct Cmp_PBRMaterial
        {
            glm::vec4 albedo = glm::vec4(1.f, 1.f, 1.f, 1.f);
            float metallic = 0.f;
            float roughness = 0.f;

            Cmp_Texture texture_albedo;
            Cmp_Texture texture_metallic;
            Cmp_Texture texture_roughness;
            Cmp_Texture texture_normal;

            Cmp_PBRMaterial(std::string ta, std::string tm, std::string tr, std::string tn)
            : texture_albedo(Cmp_Texture(ta)), texture_metallic(Cmp_Texture(tm)), texture_roughness(Cmp_Texture(tr)), texture_normal(Cmp_Texture(tn)){}
            Cmp_PBRMaterial(glm::vec4 a, float m, float r, std::string ta, std::string tm, std::string tr, std::string tn)
            : albedo(a), metallic(m), roughness(r), 
            texture_albedo(Cmp_Texture(ta)), texture_metallic(Cmp_Texture(tm)), texture_roughness(Cmp_Texture(tr)), texture_normal(Cmp_Texture(tn)){}
        };

    }
    
} // namespace Axiom
