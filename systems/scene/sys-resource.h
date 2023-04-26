#pragma once
#include "cmp-resource.h"
#include <flecs.h>

/* 
RESOURCE SYSTEM: A system for resource management
The api flow is as follows:
1. You assign an entity a resource component and a resource type component
    a. resouce component = file_path and file_name
    b. resource type = Model/Mesh, Material, Animation, etc... maybe sound and textures later
2. When its added (flecs::OnAdd) it loads the resource into the recource type's data
3. The entity will be named after its file name
4. To find the resource you just search for the entity with that name
5. There will also be helper functions to help you load an entire folder
*/

namespace axiom{
    class Sys_Resource{
        public:
            Sys_Resource();
            ~Sys_Resource();

            bool LoadPModel(flecs::entity e, Cmp_Resource& res, Cmp_Res_Model& cmp_mod);
            bool LoadPose(flecs::entity e, Cmp_Resource& res, Cmp_Res_Animations& cmp_anim);
            bool LoadDirectory(std::string directory);
            bool LoadDirectoryMulti(std::string directory);
            bool LoadMaterials(std::string file);
    };
}