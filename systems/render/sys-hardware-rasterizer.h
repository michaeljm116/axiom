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
                    void update_uniform_buffer();
                    Vulkan::VBuffer<Shader::V32> vertex_buffer;
                    Vulkan::VBuffer<uint32_t> index_buffer;
                    const std::vector<Shader::V32> vertices = {
                        {{-0.5f, -0.5f, 0.0f}, {0}, {1.0f, 0.0f, 0.0f}, {0}},
                        {{0.5f, -0.5f, 0.0f}, {0}, {0.0f, 1.0f, 0.0f}, {0}},
                        {{0.5f, 0.5f, 0.0f}, {0}, {0.0f, 0.0f, 1.0f}, {0}},
                        {{-0.5f, 0.5f, 0.0f}, {0}, {1.0f, 1.0f, 1.0f}, {0}}
                    };
                    const std::vector<uint32_t> indices = {0,1,2,2,3,0};

                    Shader::UBO ubo;
                    Vulkan::VBuffer<Shader::UBO> uniform_buffer;     // Uniform buffer object containing scene data

                    uint32_t current_frame = 0;

                    void prepare_buffers();
            };

            extern Raster g_raster;
        }
    }
}