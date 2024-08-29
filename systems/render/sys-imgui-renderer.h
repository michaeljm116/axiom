#pragma once
#include "../components/render/cmp-vulkan.h"
#include "../components/render/cmp-imgui.h"
#include "vulkan/vulkan_core.h"

static void check_vk_result(VkResult err)
	{
		if (err == 0)
			return;
		fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
		if (err < 0)
			abort();
	}

namespace Axiom{
namespace Render{
    void initialize_imgui(Cmp_Vulkan* c_v);
    void draw_imgui(VkSubmitInfo* submit_info, uint32_t image_index, uint32_t frame);
    void recreate_imgui();

    class ImGui_Renderer
    {
        public:
            ImGui_Renderer();
            ~ImGui_Renderer();

            void init(Cmp_Vulkan* c_v);
            void destroy();
            void draw(VkCommandBuffer commandBuffer, int image_index);
            void start_draw(VkSubmitInfo* submit_info, uint32_t image_index, uint32_t frame);
            void end_draw(VkSubmitInfo* submit_info, uint32_t image_index, uint32_t frame);
            inline VkCommandBuffer& command_buffer(int ii){return command_buffers.at(ii);}
            inline void recreate(){ create_command_buffers(); }
        private:
            Cmp_Vulkan* c_vulkan;

            VkDescriptorPool descriptor_pool;
            VkCommandPool command_pool;
            VkRenderPass render_pass;
            VkQueue copy_queue;

            std::vector<VkFramebuffer> frame_buffers;
            std::array<VkCommandBuffer, MAX_FRAMES_IN_FLIGHT> command_buffers;
            std::array<VkSemaphore, MAX_FRAMES_IN_FLIGHT> ui_semaphores;
		    std::array<VkClearValue,2> clear_values = {};

            void create_descriptor_pool();
            void create_render_pass();
            void create_frame_buffers();
            void create_command_pool();
            void create_command_buffers();
            void update_command_buffers(int ii);
    };
    
    extern ImGui_Renderer g_imgui;
}
}
