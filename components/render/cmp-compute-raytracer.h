/**
 * @file cmp-compute-raytracer.h
 * @author Mike Murrell(mikestoleyobike@aim.com)
 * @brief Compute Raytracer component
 * @version 0.1
 * @date 2023-06-27
 * 
 * @copyright Copyright (c) 2023
 * 
 */

 #pragma once
 #include "cmp-vulkan.h"
 #include "cmp-shader.h"
 #include "vulkan/buffer.hpp"
 #include "texture.h"
 #include "../../systems/render/texture.h"
 #include <unordered_map>



 namespace Axiom{
    namespace Render{
        
    static const int MAX_TEXTURES = 5;
        struct Cmp_ComputeRaytracer{
            bool editor = true;
            VkDescriptorPool descriptor_pool = VK_NULL_HANDLE;
            struct {
                VkDescriptorSetLayout descriptor_set_layout = VK_NULL_HANDLE;
                VkDescriptorSet descriptor_set = VK_NULL_HANDLE;
                VkPipelineLayout pipeline_layout = VK_NULL_HANDLE;
                VkPipeline pipeline = VK_NULL_HANDLE;
            }graphics;

            struct
            {
                VkQueue queue = VK_NULL_HANDLE;								    // Separate queue for compute commands (queue family may differ from the one used for graphics)
                VkCommandPool command_pool =VK_NULL_HANDLE;				    // Use a separate command pool (queue family may differ from the one used for graphics)
                VkCommandBuffer command_buffer = VK_NULL_HANDLE;				    // Command buffer storing the dispatch commands and barriers
                VkFence fence = VK_NULL_HANDLE;								    // Synchronization fence to avoid rewriting compute CB if still in use
                VkDescriptorSetLayout descriptor_set_layout = VK_NULL_HANDLE;		// Compute shader binding layout
                VkDescriptorSet descriptor_set = VK_NULL_HANDLE;				    // Compute shader bindings
                VkPipelineLayout pipeline_layout = VK_NULL_HANDLE;			    // Layout of the compute pipeline
                VkPipeline pipeline = VK_NULL_HANDLE;						    // Compute raytracing pipeline
            }compute;

            bool prepared = false;

            Axiom::Render::Texture compute_texture = Axiom::Render::Texture();
            Axiom::Render::Texture gui_textures[MAX_TEXTURES] ;

            VkDescriptorSetLayoutBinding DescriptorSetLayoutBinding(uint32_t binding, VkDescriptorType descriptorType, uint32_t descriptorCount, VkShaderStageFlags flags);
            std::vector<VkWriteDescriptorSet> compute_write_descriptor_sets;
        };
        struct Cmp_ComputeData
        {
            struct 
            {
                //GPU READ ONLY
                Vulkan::VBuffer<Shader::Vert> verts;		// (Shader) storage buffer object with scene verts
                Vulkan::VBuffer<Shader::Index> faces;		// (Shader) storage buffer object with scene indices
                Vulkan::VBuffer<Shader::BVHNode> blas;		// (Shader) storage buffer object with bottom level acceleration structure
                Vulkan::VBuffer<Shader::Shape> shapes;		// for animatied shapes 

                //CPU + GPU 
                Vulkan::VBuffer<Shader::Primitive> primitives;	// for the primitives
                Vulkan::VBuffer<Shader::Material> materials;	// (Shader) storage buffer object with scene Materials
                Vulkan::VBuffer<Shader::Light> lights;
                Vulkan::VBuffer<Shader::GUI> guis;
                Vulkan::VBuffer<Shader::BVHNode> bvh;			// for the bvh bruh
            }storage_buffers;

            struct
            {
                std::vector<Shader::Primitive> primitives;
                std::vector<Shader::Material> materials;
                std::vector<Shader::Light> lights;
                std::vector<Shader::GUI> guis;
                std::vector<Shader::BVHNode> bvh;
            }shader_data;

            struct UBOCompute{                              // Compute shader uniform block object
                glm::mat4 rotM = glm::mat4(1);
                float fov = 10.0f;
                float aspect_ratio;
                int rand;
            }ubo;
            Vulkan::VBuffer<UBOCompute> uniform_buffer;     // Uniform buffer object containing scene data

            std::vector<VkWriteDescriptorSet> write_descriptor_sets;
            std::vector<Cmp_Light*> light_comps;
            std::unordered_map<int32_t, std::pair<int, int>> mesh_assigner;    
        };
    }
 }