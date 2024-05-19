#include "sys-resource-assimp.h"
#include "sys-log.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <memory>
namespace Axiom{
    namespace Resource{
        bool load_assimp_model(flecs::entity e, Cmp_Resource& res, Cmp_ResModel& cmp_mod)
        {
            Assimp::Importer importer; 
            const auto* scene = importer.ReadFile(res.file_path,
                aiProcess_Triangulate |
                aiProcess_FlipUVs |
                aiProcess_CalcTangentSpace |
                aiProcess_JoinIdenticalVertices |
                aiProcess_SortByPType);
            
            if(!Log::check_error(scene != nullptr, "Importing " + res.file_path + res.file_name)){
                Log::send(Log::Level::ERROR, importer.GetErrorString());
                return false;
            }
            
            return false;
        }
    }
}
