#pragma once
#include "cmp-vulkan.h"
#include <imgui.h>
#include <backends/imgui_impl_vulkan.h>
#include <backends/imgui_impl_glfw.h>
#include <vector>


namespace Axiom{
namespace Render{
    struct Cmp_ImGui
    {
		VkDescriptorPool descriptor_pool;
		VkRenderPass render_pass;
		std::vector<VkCommandBuffer> command_buffers;
		VkCommandPool command_pool;
		VkSemaphore ui_semaphore;
		VkQueue copy_queue;

		std::array<VkClearValue,2> clear_values= {};
    };
}
}