#include "sys-compute-raytracer.h"

namespace Axiom{
    namespace Render{
        namespace Compute{
            std::vector<VkWriteDescriptorSet> write_descriptor_sets;
            std::vector<Cmp_Light*> light_comps;
            std::unordered_map<int32_t, std::pair<int, int>> mesh_assigner;
            ShaderData Shader_Data;
            StorageBuffers Storage_Buffers;
            
            void initialize(Cmp_ComputeRaytracer& raytracer, Cmp_Vulkan& vulkan);
            
            Raytracer::Raytracer()
            {
            }
        }
    }
}