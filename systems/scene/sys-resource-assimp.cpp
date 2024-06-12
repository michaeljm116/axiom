#include "flecs-world.h"
#include "sys-resource-assimp.h"
#include "sys-log.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <memory>
namespace Axiom{
    namespace Resource{

#pragma region helper lambdas
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

        const auto remove_file_stem = [](std::string file_name){
            int index = file_name.size();
            for(index; file_name[index] != '.'; index--)
            {
                if(index == 0) return std::string("");
            }
            return file_name.substr(0, index);
        };

        const auto set_material_name = [](aiMaterial* m, int index, Cmp_Resource res){
            std::string name = m->GetName().C_Str();
            if(name == "")
            name = "mat_" + remove_file_stem(res.file_name) + "(" + std::to_string(index) + ")";
            return name;
        };
        const auto set_mesh_name = [](aiMesh* m, int index, Cmp_Resource res){
            std::string name = m->mName.C_Str();
            if(name == "")
            name = "mesh_" + remove_file_stem(res.file_name) + "(" + std::to_string(index) + ")";
            return name;
        }; 
        const auto set_scene_name = [](const aiScene* s, Cmp_Resource res){
            std::string name = s->mName.C_Str();
            if(name == "")
                name = "scene_" + remove_file_stem(res.file_name);
            return name;
        };        


        const auto get_assimp_texture_pbr = [](aiMaterial* material, int index, Cmp_Resource res)
        {
            Resource::AxMaterial::PBR pbr;
            std::string path = res.file_path + "/";
            pbr.name = set_material_name(material, index, res);
            pbr.index = index;
            
            aiString normal_path, base_color_path, metalness_path, diffuse_rougness_path;
            aiColor4D base_color;            
            float metallic, roughness;

            if (material->GetTexture(aiTextureType_NORMALS, 0, &normal_path) == AI_SUCCESS){
                pbr.normal.file = path + normal_path.C_Str();
                pbr.normal.val = true;
            }
            if(material->GetTexture(aiTextureType_BASE_COLOR, 0, &base_color_path) == AI_SUCCESS){
                pbr.albedo.file = path + base_color_path.C_Str();
            }
            if(material->GetTexture(aiTextureType_METALNESS, 0, &metalness_path) == AI_SUCCESS){
                pbr.metalness.file = path + metalness_path.C_Str();
            }
            if(material->GetTexture(aiTextureType_DIFFUSE_ROUGHNESS, 0, &diffuse_rougness_path) == AI_SUCCESS){
                pbr.roughness.file = path + diffuse_rougness_path.C_Str();
            }
            if (aiGetMaterialColor(material, AI_MATKEY_COLOR_DIFFUSE, &base_color) == AI_SUCCESS) {
                pbr.albedo.val = glm::vec4(base_color.r, base_color.g, base_color.b, base_color.a);
            }
            if (aiGetMaterialFloat(material, AI_MATKEY_METALLIC_FACTOR, &metallic) == AI_SUCCESS) {
                pbr.metalness.val = metallic;
            }
            if (aiGetMaterialFloat(material, AI_MATKEY_ROUGHNESS_FACTOR, &roughness) == AI_SUCCESS) {
                pbr.roughness.val = roughness;
            }
            return pbr;
        };
        const auto get_assimp_texture_phong = [](aiMaterial* material, int index, Cmp_Resource res){
            Resource::AxMaterial::Phong phong;
            auto path = res.file_path + "/";
            phong.name = set_material_name(material, index, res);
            phong.index = index;
            aiString ambient_path, diffuse_path, specular_path, normal_path;
            if(material->GetTexture(aiTextureType_DIFFUSE, 0, &diffuse_path) == AI_SUCCESS){
                phong.diffuse.file = path + diffuse_path.C_Str();
            }
            if(material->GetTexture(aiTextureType_SPECULAR, 0, &specular_path) == AI_SUCCESS){
                phong.specular.file = path + specular_path.C_Str();
            }
            if(material->GetTexture(aiTextureType_AMBIENT, 0, &ambient_path) == AI_SUCCESS){
                phong.ambient.file = path + ambient_path.C_Str();
            }
            if(material->GetTexture(aiTextureType_NORMALS, 0, &normal_path) == AI_SUCCESS){
                phong.normal.file = path + normal_path.C_Str();
            }

            // Colors
            aiColor3D color;
            if (material->Get(AI_MATKEY_COLOR_DIFFUSE, color) == AI_SUCCESS) {
                phong.diffuse.val = glm::vec3(color.r, color.g, color.b);
            }
            if (material->Get(AI_MATKEY_COLOR_SPECULAR, color) == AI_SUCCESS) {
                phong.specular.val = glm::vec3(color.r, color.g, color.b);
            }
            if (material->Get(AI_MATKEY_COLOR_AMBIENT, color) == AI_SUCCESS) {
                phong.ambient.val = glm::vec3(color.r, color.g, color.b);
            }

            return phong;
        };
        #pragma endregion helper lambdas
         
        bool load_assimp_model(flecs::entity e, Cmp_Resource& res, Cmp_AssimpModel& cmp_mod)
        {
            // Load the file
            Assimp::Importer importer; 
            auto full_path = res.file_path + "/";
            const auto* scene = importer.ReadFile(full_path + res.file_name,
                aiProcess_Triangulate |
                aiProcess_FlipUVs |
                aiProcess_CalcTangentSpace |
                aiProcess_JoinIdenticalVertices |
                aiProcess_SortByPType);
            if(!Log::check_error(scene != nullptr, "Importing " + res.file_path + res.file_name)){
                Log::send(Log::Level::ERROR, importer.GetErrorString());
                return false;
            }
            
            // Get Materials
		    std::vector<Resource::AxMaterial::PBR> materials;
            auto num_materials = scene->mNumMaterials;            
            materials.reserve(num_materials);
            for(int m = 0; m < num_materials; ++m){
                materials.emplace_back(get_assimp_texture_pbr(scene->mMaterials[m], m, res));
                g_world.entity(materials[m].name.c_str()).set(materials[m]);
            }

            // Load the Data into model
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
                    .mat_name = materials[curr_mesh->mMaterialIndex].name,
                    .name = set_mesh_name(curr_mesh, m, res)
                };
                cmp_mod.subsets.emplace_back(s); 
            }

            // Define overall shape
            glm::vec3 model_max = glm::vec3(FLT_MIN);
            glm::vec3 model_min = glm::vec3(FLT_MAX);
            for(auto s : cmp_mod.subsets){
                get_max_sub(model_max, s);
                get_min_sub(model_min, s);
            }
            cmp_mod.extents = (model_max - model_min) * .5f;
            cmp_mod.center = (model_min + cmp_mod.extents);
            cmp_mod.name = set_scene_name(scene, res);
            e.set(Cmp_Assmbled());
            return true;
        }
    }
}
