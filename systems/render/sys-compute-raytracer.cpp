#include "sys-compute-raytracer.h"

namespace Axiom{
    namespace Render{
        namespace Compute{
            void initialize_raytracing(){
                /*auto vulkan_cmp = g_world.get_ref<Axiom::Render::Cmp_Vulkan>();
                auto raytrc_cmp = g_world.get_ref<Axiom::Render::Cmp_ComputeRaytracer>();
                auto compdt_cmp = g_world.get_ref<Axiom::Render::Cmp_ComputeData>();
                Raytracer raytracer(vulkan_cmp.get(), raytrc_cmp.get(), compdt_cmp.get());
                raytracer.start_up();*/
                Raytracer rt;
                rt.vulkan_component = g_world.get_ref<Axiom::Render::Cmp_Vulkan>().get();
                //rt.compute_component = g_world.get_ref<Axiom::Render::Cmp_ComputeData>().get();
                //rt.raytracing_component = g_world.get_ref<Axiom::Render::Cmp_ComputeRaytracer>().get();
            }

            Raytracer::Raytracer()
            {
            }

            Raytracer::Raytracer(Cmp_Vulkan *vk, Cmp_ComputeRaytracer *cr, Cmp_ComputeData *cd)
            {
                vulkan_component = vk;
                raytracing_component = cr;
                compute_component = cd;
            }
            void Raytracer::start_up()
            {
            }
            void Raytracer::initialize()
            {
            }
            void Raytracer::start_frame(uint32_t &image_index)
            {
            }
            void Raytracer::end_frame(const uint32_t &image_index)
            {
            }
            void Raytracer::add_entity(flecs::entity &e)
            {
            }
            void Raytracer::remove_entity(flecs::entity &e)
            {
            }
            void Raytracer::process_entity(flecs::entity &e)
            {
            }
            void Raytracer::end_update()
            {
            }
            void Raytracer::clean_up_swapchain()
            {
            }
            void Raytracer::recreate_swapchain()
            {
            }
            void Raytracer::add_material(glm::vec3 diff, float rfl, float rough, float trans, float ri)
            {
            }
            void Raytracer::update_descriptors()
            {
            }
            void Raytracer::update_material(int id)
            {
            }
            void Raytracer::toggle_playmode(bool b)
            {
            }
            void Raytracer::set_stuff_up()
            {
            }
            void Raytracer::create_graphics_pipeline()
            {
            }
            void Raytracer::create_descriptor_pool()
            {
            }
            void Raytracer::create_descriptor_sets()
            {
            }
            void Raytracer::create_descriptor_set_layout()
            {
            }
            void Raytracer::create_command_buffers(float swap_ratio, int32_t offset_width, int32_t offset_heigiht)
            {
            }
            void Raytracer::create_compute_command_buffers()
            {
            }
            void Raytracer::create_uniform_buffers()
            {
            }
            void Raytracer::prepare_storage_buffers()
            {
            }
            void Raytracer::prepare_texture_target(Texture *tex, uint32_t width, uint32_t height, VkFormat format)
            {
            }
            void Raytracer::prepare_compute()
            {
            }
            void Raytracer::destroy_compute()
            {
            }
        }
    }
}