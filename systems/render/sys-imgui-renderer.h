#pragma once
#include "../components/render/cmp-vulkan.h"
#include "../components/render/cmp-imgui.h"

static void check_vk_result(VkResult err)
	{
		if (err == 0)
			return;
		fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
		if (err < 0)
			abort();
	}

    template<typename T>
	inline void VK_CHECKRESULT(const T& res, std::string msg) {
		if (res != VK_SUCCESS)
			throw std::runtime_error("ERROR: Failed to " + msg);
	}

namespace Axiom{
namespace Render{
    void initialize_imgui(Cmp_Vulkan* c_v);
    void draw_imgui(VkSubmitInfo* submit_info, int image_index);
    class ImGui_Renderer
    {
        public: 
            ImGui_Renderer();
            ~ImGui_Renderer();

            void init(Cmp_Vulkan* c_v);
            void destroy();
            void draw(VkSubmitInfo* submit_info, int image_index);
        
        private:
            Cmp_ImGui* c_imgui;
            Cmp_Vulkan* c_vulkan;

            void create_descriptor_pool();
            void create_render_pass();
            void create_command_pool();
            void create_command_buffers();
            void update_command_buffers(int ii);
    };
    extern ImGui_Renderer g_imgui;
}
}