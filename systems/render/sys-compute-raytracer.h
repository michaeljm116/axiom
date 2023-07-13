#pragma once
#include "../components/render/cmp-compute-raytracer.h"
#include "renderer.h"

namespace Axiom
{
    namespace Render
    {
        namespace Compute
        {
            void initialize_raytracing();

            class Raytracer: public Renderer
            {
                public:
                    Raytracer();
                    Raytracer(Cmp_Vulkan* vk, Cmp_ComputeRaytracer* cr, Cmp_ComputeData* cd);
                    ~Raytracer();

                    Cmp_ComputeData* compute_component;
                    Cmp_ComputeRaytracer* raytracing_component; 
                    
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

                    void add_material(glm::vec3 diff, float rfl, float rough, float trans, float ri) override;
                    void update_descriptors() override;
                    void update_material(int id) override;
                    void toggle_playmode(bool b) override;

                private:
                    void set_stuff_up();
                    void create_graphics_pipeline();
                    void create_descriptor_pool();
                    void create_descriptor_sets();
                    void create_descriptor_set_layout();
                    void create_command_buffers(float swap_ratio,  int32_t offset_width, int32_t offset_heigiht);
                    
                    void create_compute_command_buffers();
                    void create_uniform_buffers();
                    
                    void prepare_storage_buffers();
                    void prepare_texture_target(Texture* tex, uint32_t width, uint32_t height, VkFormat format);
                    void prepare_compute();
                    void destroy_compute();
            };
        }
    }
}
