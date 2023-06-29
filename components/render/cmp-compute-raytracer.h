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

 namespace Axiom{
    namespace Render{
        struct Cmp_Compute_Raytracer{
            bool editor = true;
            VkDescriptorPool descriptor_pool;
            struct {
                VkDescriptorSetLayout descriptor_set_layout;
                VkDescriptorSet descriptor_set;
                VkPipelineLayout pipeline_layout;
                VkPipeline pipeline;
            }graphics;

            struct
                {
                struct {
                    //GPU READ ONLY
                    Vulkan::VBuffer<Shader::Vert> verts;			// (Shader) storage buffer object with scene verts
                    Vulkan::VBuffer<Shader::Index> faces;			// (Shader) storage buffer object with scene indices
                    Vulkan::VBuffer<Shader::BVHNode> blas;		// (Shader) storage buffer object with bottom level acceleration structure
                    Vulkan::VBuffer<Shader::Shape> shapes;		// for animatied shapes 

                    //CPU + GPU 
                    Vulkan::VBuffer<Shader::Primitive> primitives;	// for the primitives
                    Vulkan::VBuffer<Shader::Material> materials;	// (Shader) storage buffer object with scene Materials
                    Vulkan::VBuffer<Shader::Light> lights;
                    Vulkan::VBuffer<Shader::GUI> guis;
                    Vulkan::VBuffer<Shader::BVHNode> bvh;			// for the bvh bruh

                } storage_buffers;

                VkQueue queue;								// Separate queue for compute commands (queue family may differ from the one used for graphics)
                VkCommandPool command_pool;					// Use a separate command pool (queue family may differ from the one used for graphics)
                VkCommandBuffer command_buffer;				// Command buffer storing the dispatch commands and barriers
                VkFence fence;								// Synchronization fence to avoid rewriting compute CB if still in use
                VkDescriptorSetLayout descriptor_set_layout;	// Compute shader binding layout
                VkDescriptorSet descriptor_set;				// Compute shader bindings
                VkPipelineLayout pipeline_layout;			// Layout of the compute pipeline
                VkPipeline pipeline;						// Compute raytracing pipeline
                struct UBOCompute {							// Compute shader uniform block object
                    glm::mat4 rotM = glm::mat4(1);
                    float fov = 10.0f;
                    float aspect_ratio;
                    int rand;
                } ubo;
                Vulkan::VBuffer<UBOCompute> uniform_buffer;			// Uniform buffer object containing scene data
            } compute;

            std::vector<Shader::Primitive> primitives;
            std::vector<Shader::Material> materials;
            std::vector<Shader::Light> lights;
            std::vector<Shader::GUI> guis;
            std::vector<Shader::BVHNode> bvh;

            std::vector<MeshComponent*> mesh_comps;
            std::vector<LightComponent*> light_comps;

            std::unordered_map<int32_t, std::pair<int, int>> mesh_assigner;
            std::unordered_map<int32_t, std::pair<int, int>> joint_assigner;
            std::unordered_map<int32_t, std::pair<int, int>> shape_assigner;

            bool prepared = false;

            Texture compute_texture;
            Texture gui_textures_[MAX_TEXTURES];

            VkDescriptorSetLayoutBinding DescriptorSetLayoutBinding(uint32_t binding, VkDescriptorType descriptorType, uint32_t descriptorCount, VkShaderStageFlags flags);
            std::vector<VkWriteDescriptorSet> compute_write_descriptor_sets;
        };
    }
 }