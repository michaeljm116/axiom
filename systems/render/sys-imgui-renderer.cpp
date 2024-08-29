#include "sys-imgui-renderer.h"
#include "cmp-vulkan.h"
#include "flecs-world.h"
#include "cmp-window.h"
#include "vulkan/vulkan_core.h"

    template<typename T>
	inline void VK_CHECKRESULT(const T& res, std::string msg) {
		if (res != VK_SUCCESS)
			throw std::runtime_error("ERROR: Failed to " + msg);
	}

namespace Axiom{
namespace Render{
    ImGui_Renderer g_imgui;
    void initialize_imgui(Cmp_Vulkan *c_v)
    {
        g_imgui.init(c_v);
    }
    void draw_imgui(VkSubmitInfo* submit_info, uint32_t image_index, uint32_t frame){
        g_imgui.start_draw(submit_info, image_index, frame);
        g_imgui.end_draw(submit_info, image_index, frame);
    }
	void recreate_imgui(){
		//todo reset command buffer?
		g_imgui.recreate();
	}

    ImGui_Renderer::ImGui_Renderer()
    {
    }

    ImGui_Renderer::~ImGui_Renderer()
    {
        destroy();
    }

    void ImGui_Renderer::init(Cmp_Vulkan* c_v)
    {
        c_vulkan = c_v;
        create_descriptor_pool();
		create_render_pass();
		create_frame_buffers();
		create_command_pool();

		// Setup Dear ImGui context
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
		//io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // IF using Docking Branch

        auto* window = g_world.get<Cmp_Window>()->window;
		ImGui_ImplGlfw_InitForVulkan(window, true);
		ImGui_ImplVulkan_InitInfo* init_info = new ImGui_ImplVulkan_InitInfo();
		init_info->Instance = c_vulkan->device.instance;
		init_info->PhysicalDevice = c_vulkan->device.physical;
		init_info->Device = c_v->device.logical;
		init_info->QueueFamily = c_vulkan->device.qFams.graphicsFamily;
		init_info->Queue = c_vulkan->queues.graphics;
		init_info->PipelineCache = c_vulkan->pipeline.cache;
		init_info->DescriptorPool = descriptor_pool;
		init_info->MinImageCount = 2;
		init_info->ImageCount = 2;
		init_info->MSAASamples = c_vulkan->sample.max_sample;
		init_info->Allocator = VK_NULL_HANDLE;
		init_info->CheckVkResultFn = check_vk_result;
		init_info->RenderPass = render_pass;
		ImGui_ImplVulkan_Init(init_info);// , wd->RenderPass);
		// (this gets a bit more complicated, see example app for full reference)
		//ImGui_ImplVulkan_CreateFontsTexture(YOUR_COMMAND_BUFFER);
		// (your code submit a queue)
		//ImGui_ImplVulkan_DestroyFontUploadObjects();


		VkSemaphoreCreateInfo semaphoreInfo = {};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		for(int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
			VK_CHECKRESULT(vkCreateSemaphore(c_vulkan->device.logical, &semaphoreInfo, nullptr, &ui_semaphores[i]), " FAILED TO CREATE SEMAPHORE");
		create_command_buffers();
		copy_queue = c_vulkan->queues.graphics;
    }

    void ImGui_Renderer::destroy()
    {
		for(int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
			vkWaitForFences(c_vulkan->device.logical, 1, &c_vulkan->semaphores.presentation_fence[i], VK_TRUE, UINT64_MAX);
        ImGui_ImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
		for(int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
			vkDestroySemaphore(c_vulkan->device.logical, ui_semaphores[i], nullptr);
		vkDestroyRenderPass(c_vulkan->device.logical, render_pass, nullptr);
		vkDestroyDescriptorPool(c_vulkan->device.logical, descriptor_pool, nullptr);
		vkFreeCommandBuffers(c_vulkan->device.logical, command_pool, static_cast<uint32_t>(command_buffers.size()), command_buffers.data());
		vkDestroyCommandPool(c_vulkan->device.logical, command_pool, nullptr);
    }

    void ImGui_Renderer::draw(VkCommandBuffer commandBuffer, int image_index)
    {
        //g_imgui.start_draw(submit_info, image_index);
        //g_imgui.end_draw(submit_info, image_index);
        VkCommandBufferBeginInfo cmdBufInfo = vks::initializers::commandBufferBeginInfo();
        VkRenderPassBeginInfo renderPassBeginInfo = vks::initializers::renderPassBeginInfo();
        renderPassBeginInfo.renderPass = render_pass;
        renderPassBeginInfo.renderArea.extent.width = c_vulkan->swapchain.extent.width;
        renderPassBeginInfo.renderArea.extent.height = c_vulkan->swapchain.extent.height;
        renderPassBeginInfo.clearValueCount = static_cast<uint32_t>(clear_values.size());
        renderPassBeginInfo.pClearValues = clear_values.data();
       // renderPassBeginInfo.framebuffer = frame_buffer;

        VK_CHECKRESULT(vkBeginCommandBuffer(commandBuffer, &cmdBufInfo), "BEGIN UI COMMAND BUFFER");
        vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);


        VkViewport viewport = vks::initializers::viewport(ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.y, 0.0f, 1.0f);
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

        VkRect2D scissor = vks::initializers::rect2D((int32_t)ImGui::GetIO().DisplaySize.x, (int32_t)ImGui::GetIO().DisplaySize.y, 0, 0);
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

        ImGuiIO& io = ImGui::GetIO();
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        static bool show = true;
        ImGui::ShowDemoWindow(&show);

        ImGui::Render();
        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);

        vkCmdEndRenderPass(commandBuffer);
        VK_CHECKRESULT(vkEndCommandBuffer(commandBuffer), "END UI COMMAND BUFFER");
    }

    void ImGui_Renderer::start_draw(VkSubmitInfo* submit_info, uint32_t image_index, uint32_t frame){

		VkCommandBufferBeginInfo cmdBufInfo = vks::initializers::commandBufferBeginInfo();

		VkRenderPassBeginInfo renderPassBeginInfo = vks::initializers::renderPassBeginInfo();
		renderPassBeginInfo.renderPass = render_pass;
		renderPassBeginInfo.renderArea.extent.width = c_vulkan->swapchain.extent.width;
		renderPassBeginInfo.renderArea.extent.height = c_vulkan->swapchain.extent.height;
		renderPassBeginInfo.clearValueCount = static_cast<uint32_t>(clear_values.size());
		renderPassBeginInfo.pClearValues = clear_values.data();
		renderPassBeginInfo.framebuffer = frame_buffers[image_index];
		VK_CHECKRESULT(vkBeginCommandBuffer(command_buffers[frame], &cmdBufInfo), "BEGIN UI COMMAND BUFFER");
		vkCmdBeginRenderPass(command_buffers[frame], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    }

	void ImGui_Renderer::end_draw(VkSubmitInfo* submit_info, uint32_t image_index, uint32_t frame){
		VkViewport viewport = vks::initializers::viewport(ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.y, 0.0f, 1.0f);
		vkCmdSetViewport(command_buffers[frame], 0, 1, &viewport);

		VkRect2D scissor = vks::initializers::rect2D((int32_t)ImGui::GetIO().DisplaySize.x, (int32_t)ImGui::GetIO().DisplaySize.y, 0, 0);
		vkCmdSetScissor(command_buffers[frame], 0, 1, &scissor);


		ImGuiIO& io = ImGui::GetIO();
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		static bool show = true;
        ImGui::ShowDemoWindow(&show);


        ImGui::Render();
		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), command_buffers[frame]);

		vkCmdEndRenderPass(command_buffers[frame]);
		VK_CHECKRESULT(vkEndCommandBuffer(command_buffers[frame]), "END UI COMMAND BUFFER");

		// Submit to Queue
        VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
		submit_info->waitSemaphoreCount = 1;
		submit_info->pWaitSemaphores = submit_info->pSignalSemaphores;
		submit_info->signalSemaphoreCount = 1;
		submit_info->pSignalSemaphores = &ui_semaphores[frame];
		submit_info->commandBufferCount = 1;
		submit_info->pCommandBuffers = &command_buffers[frame];
		submit_info->pWaitDstStageMask = waitStages;
		auto result = vkQueueSubmit(c_vulkan->queues.graphics, 1, submit_info, VK_NULL_HANDLE);// "UI QUEUE SUBMIT");
		VK_CHECKRESULT(result, "UI QUEUE SUBMIT");
    }

	void ImGui_Renderer::create_descriptor_pool()
    {
		//create the imgui descriptor pool
		VkDescriptorPoolSize pool_sizes[] = { { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 } };

		VkDescriptorPoolCreateInfo pool_info = {};
		pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		pool_info.maxSets = 1000;
		pool_info.poolSizeCount = (uint32_t)std::size(pool_sizes);
		pool_info.pPoolSizes = pool_sizes;

		VK_CHECKRESULT(vkCreateDescriptorPool(c_vulkan->device.logical, &pool_info, nullptr, &descriptor_pool), "CREATING IMGUI DESCRIPTOR POOL");
    }

    void ImGui_Renderer::create_render_pass()
    {
		std::array<VkAttachmentDescription, 3> attachments = {};
		// Color attachment
		attachments[0].format = c_vulkan->swapchain.image_format;
		attachments[0].samples = c_vulkan->sample.max_sample;
		attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
		attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachments[0].initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		// Depth attachment
		attachments[1].format = c_vulkan->depth.format;
		attachments[1].samples = c_vulkan->sample.max_sample;
		attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		// Color Resolve attachment
		attachments[2].format = c_vulkan->swapchain.image_format;
		attachments[2].samples = VK_SAMPLE_COUNT_1_BIT;
		attachments[2].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachments[2].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachments[2].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachments[2].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachments[2].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attachments[2].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		std::array<VkAttachmentReference,3> references = {};
		//Color Reference
		references[0].attachment = 0;
		references[0].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		//Depth Reference
		references[1].attachment = 1;
		references[1].layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		//Resolve Reference
		references[2].attachment = 2;
		references[2].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass = {};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS; //let it know its a graphics subpass @COMPUTEHERE
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &references.at(0);
        subpass.pDepthStencilAttachment = &references.at(1);
        subpass.pResolveAttachments = &references.at(2);

		std::array<VkSubpassDependency,2> subpassDependencies = {};
		// Transition from final to initial (VK_SUBPASS_EXTERNAL refers to all commmands executed outside of the actual renderpass)
		subpassDependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
		subpassDependencies[0].dstSubpass = 0;
		subpassDependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		subpassDependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		subpassDependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		subpassDependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		subpassDependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		// Transition from initial to final
		subpassDependencies[1].srcSubpass = 0;
		subpassDependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
		subpassDependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		subpassDependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		subpassDependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		subpassDependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		subpassDependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		VkRenderPassCreateInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.pNext = NULL;
		renderPassInfo.attachmentCount = 3;
		renderPassInfo.pAttachments = attachments.data();
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 2;
		renderPassInfo.pDependencies = subpassDependencies.data();

		VK_CHECKRESULT(vkCreateRenderPass(c_vulkan->device.logical, &renderPassInfo, nullptr, &render_pass), "CREATE UI RENDER PASS");
    }

    void ImGui_Renderer::create_frame_buffers()
    {
		size_t num_frames = c_vulkan->swapchain.image_views.size();
		frame_buffers.resize(num_frames);
		for(int i = 0; i < num_frames; ++i){
			std::array<VkImageView,3> attachments = {
                c_vulkan->sample.image_view,
                c_vulkan->depth.image_view,
                c_vulkan->swapchain.image_views[i]
			};

			VkFramebufferCreateInfo frame_buffer_info = {
				.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
				.renderPass = render_pass,
				.attachmentCount = 3,
				.pAttachments = attachments.data(),
				.width = c_vulkan->swapchain.extent.width,
				.height = c_vulkan->swapchain.extent.height,
				.layers = 1
			};
			VK_CHECKRESULT(vkCreateFramebuffer(c_vulkan->device.logical, &frame_buffer_info, nullptr, &frame_buffers[i]), "CREATE UI FRAMEBUFFER");
		}
    }

    void ImGui_Renderer::create_command_pool()
    {
		VkCommandPoolCreateInfo cmdPoolInfo = {};
		cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		cmdPoolInfo.queueFamilyIndex = c_vulkan->device.qFams.graphicsFamily;
		cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		VK_CHECKRESULT(vkCreateCommandPool(c_vulkan->device.logical, &cmdPoolInfo, nullptr, &command_pool), "CREATE UI COMMAND POOL");
    }

    void ImGui_Renderer::create_command_buffers()
    {
		VkCommandBufferAllocateInfo cmdBufAllocateInfo =
		vks::initializers::commandBufferAllocateInfo(command_pool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, static_cast<uint32_t>(command_buffers.size()));
		VK_CHECKRESULT(vkAllocateCommandBuffers(c_vulkan->device.logical, &cmdBufAllocateInfo, command_buffers.data()), "ALLOCATE UI COMMADN BUFFERS");

		clear_values[0].color = { 0.0f, 0.0f, 0.0f, 1.0f }; //derp
		clear_values[1].depthStencil = { 1.0f, 0 }; //1.0 = farplane, 0.0 = nearplane HELP

		VkCommandBufferBeginInfo cmdBufInfo = vks::initializers::commandBufferBeginInfo();

		VkRenderPassBeginInfo renderPassBeginInfo = vks::initializers::renderPassBeginInfo();
		renderPassBeginInfo.renderPass = render_pass;
		renderPassBeginInfo.renderArea.extent.width = c_vulkan->swapchain.extent.width;
		renderPassBeginInfo.renderArea.extent.height = c_vulkan->swapchain.extent.height;
		renderPassBeginInfo.clearValueCount = static_cast<uint32_t>(clear_values.size());
		renderPassBeginInfo.pClearValues = clear_values.data();

		ImGuiIO& io = ImGui::GetIO();

		for (size_t i = 0; i < command_buffers.size(); ++i) {
			renderPassBeginInfo.framebuffer = frame_buffers[i];

			VK_CHECKRESULT(vkBeginCommandBuffer(command_buffers[i], &cmdBufInfo), "BEGIN UI COMMAND BUFFER");
			vkCmdBeginRenderPass(command_buffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
			ImGui_ImplVulkan_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();
			ImGui::ShowDemoWindow(); // Show demo window! :)
			// (Your code clears your framebuffer, renders your other stuff etc.)
			VkViewport viewport = vks::initializers::viewport(ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.y, 0.0f, 1.0f);
			vkCmdSetViewport(command_buffers[i], 0, 1, &viewport);

			VkRect2D scissor = vks::initializers::rect2D((int32_t)ImGui::GetIO().DisplaySize.x, (int32_t)ImGui::GetIO().DisplaySize.y, 0, 0);
			vkCmdSetScissor(command_buffers[i], 0, 1, &scissor);
			ImGui::Render();
			ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), command_buffers[i]);
			// (Your code calls vkCmdEndRenderPass, vkQueueSubmit, vkQueuePresentKHR etc.)
			vkCmdEndRenderPass(command_buffers[i]);
			VK_CHECKRESULT(vkEndCommandBuffer(command_buffers[i]), "END UI COMMAND BUFFER");
		}

    }

}
}
