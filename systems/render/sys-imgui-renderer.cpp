#include "sys-imgui-renderer.h"
#include "flecs-world.h"
#include "cmp-window.h"

namespace Axiom{
namespace Render{
    ImGui_Renderer g_imgui;
    void initialize_imgui(Cmp_Vulkan *c_v)
    {
        g_imgui.init(c_v);
    }
    void draw_imgui(VkSubmitInfo* submit_info, int image_index)
    {
        g_imgui.draw(submit_info, image_index);
    }

    ImGui_Renderer::ImGui_Renderer()
    {
        c_imgui = new Cmp_ImGui();
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
		ImGui_ImplVulkan_InitInfo init_info = {};
		init_info.Instance = c_vulkan->device.instance;
		init_info.PhysicalDevice = c_vulkan->device.physical;
		init_info.Device = c_vulkan->device.logical;
		init_info.QueueFamily = c_vulkan->device.qFams.graphicsFamily;
		init_info.Queue = c_vulkan->queues.graphics;
		init_info.PipelineCache = c_vulkan->pipeline.cache;
		init_info.DescriptorPool = c_imgui->descriptor_pool;
		init_info.Subpass = 0;
		init_info.MinImageCount = 2;
		init_info.ImageCount = 2;
		init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
		init_info.Allocator = VK_NULL_HANDLE;
		init_info.CheckVkResultFn = check_vk_result;
		//init_info.RenderPass = c_imgui->render_pass;
		ImGui_ImplVulkan_Init(&init_info, c_imgui->render_pass);// , wd->RenderPass);
		// (this gets a bit more complicated, see example app for full reference)
		//ImGui_ImplVulkan_CreateFontsTexture(YOUR_COMMAND_BUFFER);
		// (your code submit a queue)
		//ImGui_ImplVulkan_DestroyFontUploadObjects();


		VkSemaphoreCreateInfo semaphoreInfo = {};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		VK_CHECKRESULT(vkCreateSemaphore(c_vulkan->device.logical, &semaphoreInfo, nullptr, &c_imgui->ui_semaphore), " FAILED TO CREATE SEMAPHORE");


		create_command_buffers();
		c_imgui->copy_queue = c_vulkan->queues.copy;
    }

    void ImGui_Renderer::destroy()
    {
        ImGui_ImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
		vkDestroySemaphore( c_vulkan->device.logical, c_imgui->ui_semaphore, nullptr);
		vkDestroyRenderPass(c_vulkan->device.logical, c_imgui->render_pass, nullptr);
		vkDestroyDescriptorPool(c_vulkan->device.logical, c_imgui->descriptor_pool, nullptr);
		vkFreeCommandBuffers(c_vulkan->device.logical, c_imgui->command_pool, static_cast<uint32_t>(c_imgui->command_buffers.size()), c_imgui->command_buffers.data());
		vkDestroyCommandPool(c_vulkan->device.logical, c_imgui->command_pool, nullptr);
    }

    void ImGui_Renderer::draw(VkSubmitInfo *submitInfo, int image_index)
    {
        		VkPipelineStageFlags flags = 0x00000200;
		const VkPipelineStageFlags* cf = &flags;

		// Rendering


		submitInfo->waitSemaphoreCount = 1;
		submitInfo->pWaitSemaphores = submitInfo->pSignalSemaphores;
		submitInfo->signalSemaphoreCount = 1;
		submitInfo->pSignalSemaphores = &c_imgui->ui_semaphore;
		submitInfo->commandBufferCount = 1;
		submitInfo->pCommandBuffers = &c_imgui->command_buffers[image_index];
		submitInfo->pWaitDstStageMask = cf;


		update_command_buffers(image_index);
		VK_CHECKRESULT(vkQueueSubmit(c_vulkan->queues.copy, 1, submitInfo, VK_NULL_HANDLE), "UI QUEUE SUBMIT");
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

		
		VK_CHECKRESULT(vkCreateDescriptorPool(c_vulkan->device.logical, &pool_info, nullptr, &c_imgui->descriptor_pool), "CREATING IMGUI DESCRIPTOR POOL");
    }

    void ImGui_Renderer::create_render_pass()
    {
		VkAttachmentDescription attachments[2] = {};

		// Color attachment
		attachments[0].format = c_vulkan->swapchain.image_format;
		attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
		attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
		attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachments[0].initialLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		// Depth attachment
		attachments[1].format = c_vulkan->depth.format;
		attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
		attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentReference colorReference = {};
		colorReference.attachment = 0;
		colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference depthReference = {};
		depthReference.attachment = 1;
		depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkSubpassDependency subpassDependencies[2] = {};

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

		VkSubpassDescription subpassDescription = {};
		subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpassDescription.flags = 0;
		subpassDescription.inputAttachmentCount = 0;
		subpassDescription.pInputAttachments = NULL;
		subpassDescription.colorAttachmentCount = 1;
		subpassDescription.pColorAttachments = &colorReference;
		subpassDescription.pResolveAttachments = NULL;
		subpassDescription.pDepthStencilAttachment = &depthReference;
		subpassDescription.preserveAttachmentCount = 0;
		subpassDescription.pPreserveAttachments = NULL;

		VkRenderPassCreateInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.pNext = NULL;
		renderPassInfo.attachmentCount = 2;
		renderPassInfo.pAttachments = attachments;
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpassDescription;
		renderPassInfo.dependencyCount = 2;
		renderPassInfo.pDependencies = subpassDependencies;

		VK_CHECKRESULT(vkCreateRenderPass(c_vulkan->device.logical, &renderPassInfo, nullptr, &c_imgui->render_pass), "CREATE UI RENDER PASS");
    }

    void ImGui_Renderer::create_command_pool()
    {
		VkCommandPoolCreateInfo cmdPoolInfo = {};
		cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		cmdPoolInfo.queueFamilyIndex = c_vulkan->device.qFams.graphicsFamily;
		cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		VK_CHECKRESULT(vkCreateCommandPool(c_vulkan->device.logical, &cmdPoolInfo, nullptr, &c_imgui->command_pool), "CREATE UI COMMAND POOL");
    }

    void ImGui_Renderer::create_command_buffers()
    {
		c_imgui->command_buffers.resize(c_vulkan->swapchain.frame_buffers.size());
		VkCommandBufferAllocateInfo cmdBufAllocateInfo =
		vks::initializers::commandBufferAllocateInfo(c_imgui->command_pool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, static_cast<uint32_t>(c_imgui->command_buffers.size()));
		VK_CHECKRESULT(vkAllocateCommandBuffers(c_vulkan->device.logical, &cmdBufAllocateInfo, c_imgui->command_buffers.data()), "ALLOCATE UI COMMADN BUFFERS");

		c_imgui->clear_values[0].color = { 0.0f, 0.0f, 0.0f, 1.0f }; //derp
		c_imgui->clear_values[1].depthStencil = { 1.0f, 0 }; //1.0 = farplane, 0.0 = nearplane HELP

		VkCommandBufferBeginInfo cmdBufInfo = vks::initializers::commandBufferBeginInfo();

		VkRenderPassBeginInfo renderPassBeginInfo = vks::initializers::renderPassBeginInfo();
		renderPassBeginInfo.renderPass = c_imgui->render_pass;
		renderPassBeginInfo.renderArea.extent.width = c_vulkan->swapchain.extent.width;
		renderPassBeginInfo.renderArea.extent.height = c_vulkan->swapchain.extent.height;
		renderPassBeginInfo.clearValueCount = static_cast<uint32_t>(c_imgui->clear_values.size());
		renderPassBeginInfo.pClearValues = c_imgui->clear_values.data();

		ImGuiIO& io = ImGui::GetIO();

		for (size_t i = 0; i < c_imgui->command_buffers.size(); ++i) {
			renderPassBeginInfo.framebuffer = c_vulkan->swapchain.frame_buffers[i];

			VK_CHECKRESULT(vkBeginCommandBuffer(c_imgui->command_buffers[i], &cmdBufInfo), "BEGIN UI COMMAND BUFFER");
			vkCmdBeginRenderPass(c_imgui->command_buffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
			ImGui_ImplVulkan_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();
			ImGui::ShowDemoWindow(); // Show demo window! :)
			// (Your code clears your framebuffer, renders your other stuff etc.)
			VkViewport viewport = vks::initializers::viewport(ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.y, 0.0f, 1.0f);
			vkCmdSetViewport(c_imgui->command_buffers[i], 0, 1, &viewport);

			VkRect2D scissor = vks::initializers::rect2D((int32_t)ImGui::GetIO().DisplaySize.x, (int32_t)ImGui::GetIO().DisplaySize.y, 0, 0);
			vkCmdSetScissor(c_imgui->command_buffers[i], 0, 1, &scissor);
			ImGui::Render();
			ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), c_imgui->command_buffers[i]);
			// (Your code calls vkCmdEndRenderPass, vkQueueSubmit, vkQueuePresentKHR etc.)
			vkCmdEndRenderPass(c_imgui->command_buffers[i]);
			VK_CHECKRESULT(vkEndCommandBuffer(c_imgui->command_buffers[i]), "END UI COMMAND BUFFER");
		}
		
    }

    void ImGui_Renderer::update_command_buffers(int ii)
    {
		VkCommandBufferBeginInfo cmdBufInfo = vks::initializers::commandBufferBeginInfo();

		VkRenderPassBeginInfo renderPassBeginInfo = vks::initializers::renderPassBeginInfo();
		renderPassBeginInfo.renderPass = c_imgui->render_pass;
		renderPassBeginInfo.renderArea.extent.width = c_vulkan->swapchain.extent.width;
		renderPassBeginInfo.renderArea.extent.height = c_vulkan->swapchain.extent.height;
		renderPassBeginInfo.clearValueCount = static_cast<uint32_t>(c_imgui->clear_values.size());
		renderPassBeginInfo.pClearValues = c_imgui->clear_values.data();

		ImGuiIO& io = ImGui::GetIO();

		renderPassBeginInfo.framebuffer = c_vulkan->swapchain.frame_buffers[ii];
		VK_CHECKRESULT(vkBeginCommandBuffer(c_imgui->command_buffers[ii], &cmdBufInfo), "BEGIN UI COMMAND BUFFER");
		vkCmdBeginRenderPass(c_imgui->command_buffers[ii], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);



		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		static bool show = true;
		ImGui::ShowDemoWindow(&show); // Show demo window! :)
		// (Your code clears your framebuffer, renders your other stuff etc.)
		VkViewport viewport = vks::initializers::viewport(ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.y, 0.0f, 1.0f);
		vkCmdSetViewport(c_imgui->command_buffers[ii], 0, 1, &viewport);

		VkRect2D scissor = vks::initializers::rect2D((int32_t)ImGui::GetIO().DisplaySize.x, (int32_t)ImGui::GetIO().DisplaySize.y, 0, 0);
		vkCmdSetScissor(c_imgui->command_buffers[ii], 0, 1, &scissor);


		ImGui::Render();
		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), c_imgui->command_buffers[ii]);
		// (Your code calls vkCmdEndRenderPass, vkQueueSubmit, vkQueuePresentKHR etc.)


		vkCmdEndRenderPass(c_imgui->command_buffers[ii]);
		VK_CHECKRESULT(vkEndCommandBuffer(c_imgui->command_buffers[ii]), "END UI COMMAND BUFFER");
    }
}
}