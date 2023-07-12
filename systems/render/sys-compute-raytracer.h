#pragma once
#include "../components/render/cmp-compute-raytracer.h"
#include "renderer.h"

namespace Axiom
{
    namespace Render
    {
        namespace Compute
        {
            struct StorageBuffers 
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
            };
            struct ShaderData
            {
                std::vector<Shader::Primitive> primitives;
                std::vector<Shader::Material> materials;
                std::vector<Shader::Light> lights;
                std::vector<Shader::GUI> guis;
                std::vector<Shader::BVHNode> bvh;
            };

            extern std::vector<VkWriteDescriptorSet> write_descriptor_sets;
            extern std::vector<Cmp_Light*> light_comps;
            extern std::unordered_map<int32_t, std::pair<int, int>> mesh_assigner;     
            extern StorageBuffers Storage_Buffers;
            extern ShaderData Shader_Data;
            extern Raytracer raytracer;

            void initialize(Cmp_ComputeRaytracer& c_raytracer, Cmp_Vulkan& vulkan);

            class Raytracer: public Renderer
            {
                public:
                    Raytracer();
                    ~Raytracer();
                    
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
