#pragma once
#include "render-base.h"
#include "../../components/render/cmp-shader.h"
#include "../../components/render/cmp-camera.h"
namespace Axiom
{
    namespace Render
    {
        class Renderer: public Base::RenderBase
        {
            public: 
            Renderer() {};
            virtual ~Renderer() {};

            virtual void start_up() = 0;
            virtual void initialize() = 0;
            virtual void start_frame(uint32_t& image_index) = 0;
            virtual void end_frame(const uint32_t& image_index) = 0;

            virtual void add_entity(flecs::entity& e) = 0;
            virtual void remove_entity(flecs::entity& e) = 0;
            virtual void process_entity(flecs::entity& e) = 0;
            virtual void end_update() = 0;

            virtual void clean_up() = 0;// { RenderBase::cleanup(); }
            virtual void clean_up_swapchain() = 0;// { RenderBase::cleanupSwapChain(); }
            virtual void recreate_swapchain() = 0;// { RenderBase::recreateSwapChain(); }

            virtual void add_material(glm::vec3 diff, float rfl, float rough, float trans, float ri) = 0;
            virtual void update_descriptors() = 0;
            virtual void update_material(std::string name) = 0;   
            virtual void update_camera(Cmp_Camera* c) = 0;
            virtual void toggle_playmode(bool b) = 0;

            void update_deviceinfo();

            enum RendererUpdateFlags{
                kUpdateBox = 0x01,
                kUpdateSphere = 0x02,
                kUpdatePlane = 0x04,
                kUpdateCylinder = 0x08,
                kUpdateCone = 0x10,
                kUpdateMesh = 0x20,
                kUpdateMaterial = 0x40,
                kUpdateNone = 0x80,
                kUpdateObject = 0x100,
                kUpdateLight = 0x200,
                kUpdateGui = 0x400,
                kUpdateBvh = 0x800,
            };
            int32_t update_flags;
            inline void update_renderer(RendererUpdateFlags flag){
                update_flags |= flag;
                if(update_flags & kUpdateNone)
                    update_flags &= ~kUpdateNone;
            }

        };
    }
}