#include "flecs-world.h"
#include "sys-resource-assimp.h"
#include "sys-log.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <memory>
namespace Axiom{
    namespace Resource{

        const auto to_Vertex64 = [](const aiVector3D& p, const aiVector3D& n, const aiVector3D& t, const aiVector3D& uv){
            glm::vec3 pos = glm::vec3(p.x, p.y, p.z);
            glm::vec3 norm = glm::vec3(n.x, n.y, n.z);
            glm::vec3 tang = glm::vec3(t.x, t.y, t.z);
            glm::vec2 txtr = glm::vec2(uv.x, uv.y);
            return Vertex64(pos, norm, tang, txtr);
        };

        const auto get_min = [](glm::vec3& min_vec, const aiVector3D& comp_vec){
            if(comp_vec.x < min_vec.x) min_vec.x = comp_vec.x;
            if(comp_vec.y < min_vec.y) min_vec.y = comp_vec.y;
            if(comp_vec.z < min_vec.z) min_vec.z = comp_vec.z;
        };
        const auto get_max = [](glm::vec3& max_vec, const aiVector3D& comp_vec){
            if(comp_vec.x > max_vec.x) max_vec.x = comp_vec.x;
            if(comp_vec.y > max_vec.y) max_vec.y = comp_vec.y;
            if(comp_vec.z > max_vec.z) max_vec.z = comp_vec.z;
        };

        const auto get_min_sub = [](glm::vec3& min_vec, const Subset& subset){
            auto comp_vec = subset.center - subset.extents;
            if(comp_vec.x < min_vec.x) min_vec.x = comp_vec.x;
            if(comp_vec.y < min_vec.y) min_vec.y = comp_vec.y;
            if(comp_vec.z < min_vec.z) min_vec.z = comp_vec.z;
        };
        const auto get_max_sub = [](glm::vec3& max_vec, const Subset& subset){
            auto comp_vec = subset.center + subset.extents;
            if(comp_vec.x > max_vec.x) max_vec.x = comp_vec.x;
            if(comp_vec.y > max_vec.y) max_vec.y = comp_vec.y;
            if(comp_vec.z > max_vec.z) max_vec.z = comp_vec.z;
        };

        const auto get_assimp_textures = [](aiMaterial* material, int index){
            Resource::Material;
            aiString diffuse_path;
            aiString specular_path;
            aiString normal_path;
            aiString base_color_path;
            aiString metalness_path;
            aiString diffuse_rougness_path;
            bool has_diffuse = false;
            bool has_base_color = false;
            bool has_metalness = false;
            bool has_roughness = false;
            if(material->GetTexture(aiTextureType_DIFFUSE, 0, &diffuse_path) == AI_SUCCESS)
                has_diffuse = true;
            if(material->GetTexture(aiTextureType_BASE_COLOR, 0, &base_color_path) == AI_SUCCESS)
                has_base_color = true;
            if(material->GetTexture(aiTextureType_METALNESS, 0, &metalness_path) == AI_SUCCESS)
                has_metalness = true;
            if(material->GetTexture(aiTextureType_DIFFUSE_ROUGHNESS, 0, &diffuse_rougness_path) == AI_SUCCESS)
                has_roughness = true;
            auto name = material->GetName().C_Str();
        
            
        };

        bool load_assimp_model(flecs::entity e, Cmp_Resource& res, Cmp_AssimpModel& cmp_mod)
        {
            Assimp::Importer importer; 
            const auto* scene = importer.ReadFile(res.file_path + "/" + res.file_name,
                aiProcess_Triangulate |
                aiProcess_FlipUVs |
                aiProcess_CalcTangentSpace |
                aiProcess_JoinIdenticalVertices |
                aiProcess_SortByPType);
            
            if(!Log::check_error(scene != nullptr, "Importing " + res.file_path + res.file_name)){
                Log::send(Log::Level::ERROR, importer.GetErrorString());
                return false;
            }
            auto num_meshes = scene->mNumMeshes;
            cmp_mod.subsets.reserve(num_meshes);
            for(int m = 0; m < num_meshes; ++m)
            {
                const auto& curr_mesh = scene->mMeshes[m];
                glm::vec3 max_vert = glm::vec3(FLT_MIN);
                glm::vec3 min_vert = glm::vec3(FLT_MAX);
                const auto num_verts = curr_mesh->mNumVertices;
                std::vector<Vertex64> vertices;
                vertices.reserve(num_verts);
                for(int v = 0; v < num_verts; ++v){
                    const auto& curr_vert = curr_mesh->mVertices[v];
                    const auto& curr_norm = curr_mesh->HasNormals() ? curr_mesh->mNormals[v] : aiVector3D(0);
                    const auto& curr_tang = curr_mesh->HasTangentsAndBitangents() ? curr_mesh->mTangents[v] : aiVector3D(0);
                    auto text_cord = curr_mesh->HasTextureCoords(0) ? curr_mesh->mTextureCoords[0][v] : aiVector3D(0);
                    vertices.emplace_back(to_Vertex64(curr_vert, curr_norm, curr_tang, text_cord));
                    get_max(max_vert, curr_vert);
                    get_min(min_vert, curr_vert);
                }

                glm::vec3 extents = (max_vert - min_vert) * .5f;
                glm::vec3 center = min_vert + extents;

                const auto num_faces = curr_mesh->mNumFaces;
                std::vector<glm::ivec3> tris;
                tris.reserve(num_faces);
                for(auto f = 0; f < num_faces; ++f){
                    glm::ivec3 face = glm::ivec3(curr_mesh->mFaces[f].mIndices[0], curr_mesh->mFaces[f].mIndices[1], curr_mesh->mFaces[f].mIndices[2]);
                    tris.emplace_back(face);
                }

                Subset s = {
                    .verts = vertices,
                    .tris = tris,
                    .center = center,
                    .extents = extents,
                    .mat_id = curr_mesh->mMaterialIndex,
                    .name = curr_mesh->mName.C_Str()
                };
                cmp_mod.subsets.emplace_back(s); 
            }

            glm::vec3 model_max = glm::vec3(FLT_MIN);
            glm::vec3 model_min = glm::vec3(FLT_MAX);
            for(auto s : cmp_mod.subsets){
                get_max_sub(model_max, s);
                get_min_sub(model_min, s);
            }

            cmp_mod.extents = (model_max - model_min) * .5f;
            cmp_mod.center = (model_min + cmp_mod.extents);
            cmp_mod.name = scene->mName.C_Str();
            //For eaach material
            //scene->mMaterials[0]->mProperties[0];
            
            auto num_materials = scene->mNumMaterials;
            std::vector<Resource::Material> mats; 
            mats.reserve(num_materials);
            for(int m = 0; m < num_materials; ++m){
                get_assimp_textures(scene->mMaterials[m], m);
            }
            return false;
        }
    }
}
