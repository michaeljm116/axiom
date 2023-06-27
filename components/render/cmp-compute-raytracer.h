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

 namespace axiom{
    namespace render{
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
                    vulkan::VBuffer<shader::Vert> verts;			// (Shader) storage buffer object with scene verts
                    vulkan::VBuffer<shader::Index> faces;			// (Shader) storage buffer object with scene indices
                    vulkan::VBuffer<shader::BVHNode> blas;		// (Shader) storage buffer object with bottom level acceleration structure
                    vulkan::VBuffer<shader::Shape> shapes;		// for animatied shapes 

                    //CPU + GPU 
                    vulkan::VBuffer<shader::Primitive> primitives;	// for the primitives
                    vulkan::VBuffer<shader::Material> materials;	// (Shader) storage buffer object with scene Materials
                    vulkan::VBuffer<shader::Light> lights;
                    vulkan::VBuffer<shader::GUI> guis;
                    vulkan::VBuffer<shader::BVHNode> bvh;			// for the bvh bruh

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
                vulkan::VBuffer<UBOCompute> uniform_buffer;			// Uniform buffer object containing scene data
            } compute;

            std::vector<shader::Primitive> primitives;
            std::vector<shader::Material> materials;
            std::vector<shader::Light> lights;
            std::vector<shader::GUI> guis;
            std::vector<shader::BVHNode> bvh;

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