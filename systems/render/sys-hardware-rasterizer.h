/**
 * @file sys-hardware-rasterizer.h
 * @author Mike Murrell(mikestoleyobike@aim.com)
 * @brief Compute Raytracer component
 * @version 0.1
 * @date 2024-05-04
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#pragma once
#include "../components/render/cmp-hardware-rasterizer.h"
#include "renderer.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Axiom
{
    namespace Render
    {
        namespace Hardware
        {
            void initialize_raster();
            class Raster: public Renderer
            {
                public: 
                    Raster();
                    Raster(Cmp_Vulkan* vk, Cmp_GraphicsPipeline* gp);
                    ~Raster();

                    Cmp_GraphicsPipeline* graphics_pipeline;

                    void start_up() override;
                    void initialize() override;
                    void draw_frame();
                    void start_frame(uint32_t& image_index) override;
                    void end_frame(const uint32_t& image_index) override;

                    void add_entity(flecs::entity& e) override;
                    void remove_entity(flecs::entity& e) override;
                    void process_entity(flecs::entity& e) override;
                    void end_update() override;

                    void clean_up() override;
                    void clean_up_swapchain() override;
                    void recreate_swapchain() override;
                    void toggle_playmode(bool b) override;

                    void add_material(glm::vec3 diff, float rfl, float rough, float trans, float ri) override;
                    void update_material(std::string name) override;
		            void update_camera(Cmp_Camera* c) override;
                    void update_descriptors() override;

                private:
                    void create_graphics_pipeline();
                    void create_descriptor_pool();
                    void create_descriptor_sets();
                    void create_descriptor_set_layout();
                    void create_command_buffers(float swap_ratio,  int32_t offset_width, int32_t offset_heigiht);  
                    void update_command_buffer(VkCommandBuffer command_buffer, uint32_t image_index);
                    void update_uniform_buffer(uint32_t current_frame);
                    
                    
                    Vulkan::VBuffer<Shader::V32> vertex_buffer;
                    Vulkan::VBuffer<uint32_t> index_buffer;
                    const std::vector<Shader::V32> vertices = {
                        {{-0.5f, -0.5f, 0.5f}, {1.f}, {1.0f, 0.0f, 0.0f}, {0}},
                        {{0.5f, -0.5f, 0.5f}, {0}, {0.0f, 1.0f, 0.0f}, {0}},
                        {{0.5f, 0.5f, 0.5f}, {0}, {0.0f, 0.0f, 1.0f}, {1.f}},
                        {{-0.5f, 0.5f, 0.5f}, {1.f}, {1.0f, 1.0f, 1.0f}, {1.f}},

                        {{-0.5f, -0.5f, -.50f}, {1.f}, {1.0f, 0.0f, 0.0f}, {0}},
                        {{0.5f, -0.5f, -.50f}, {0}, {0.0f, 1.0f, 0.0f}, {0}},
                        {{0.5f, 0.5f, -.50f}, {0}, {0.0f, 0.0f, 1.0f}, {1.f}},
                        {{-0.5f, 0.5f, -.50f}, {1.f}, {1.0f, 1.0f, 1.0f}, {1.f}}

                    };
                    const std::vector<uint32_t> indices = {
                        // Front face
                        0, 1, 2,
                        2, 3, 0,

                        // Back face
                        4, 5, 6,
                        6, 7, 4,

                        // Top face
                        3, 2, 6,
                        6, 7, 3,

                        // Bottom face
                        4, 5, 1,
                        1, 0, 4,

                        // Right face
                        1, 5, 6,
                        6, 2, 1,

                        // Left face
                        4, 0, 3,
                        3, 7, 4
                    };


                    Shader::UBO ubo = {
                        .model = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
                        .view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
                        .proj = glm::perspective(glm::radians(45.0f), 1.7777777f , 0.1f, 10.0f)
                    };
                    std::array<Vulkan::VBuffer<Shader::UBO>, MAX_FRAMES_IN_FLIGHT> uniform_buffers;     // Uniform buffer object containing scene data

                    uint32_t current_frame = 0;
                    Texture texture;

                    void prepare_buffers();
            };

            extern Raster g_raster;
        }
    }
}