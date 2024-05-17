#include "sys-hardware-rasterizer.h"
#include "sys-shader.h"
#include "sys-log.h"
#include <array>
#include "cmp-resource.h"


namespace Axiom{
    namespace Render{
        namespace Hardware{
            Raster::Raster(){

            }
            Raster::~Raster(){}

            Raster g_raster = Raster();
            void initialize_raster(){
                g_raster.vulkan_component = g_world.get_ref<Axiom::Render::Cmp_Vulkan>().get();
                g_raster.graphics_pipeline = g_world.get_ref<Axiom::Render::Cmp_GraphicsPipeline>().get();
                g_raster.start_up();
                g_raster.initialize();
            }
            void Raster::start_up()
            {
                initVulkan();
            }

            void Raster::initialize()
            {

                create_graphics_pipeline();
                
                create_command_buffers(1.f, 0, 0);
            }
            
            void Raster::start_frame(uint32_t &image_index)
            {
                auto result = vkAcquireNextImageKHR(vulkan_component->device.logical, vulkan_component->swapchain.get,
                                                    std::numeric_limits<uint64_t>::max(), 
                                                    vulkan_component->semaphores.image_available, VK_NULL_HANDLE, &image_index);
                if(!Log::check_error(result == VK_SUCCESS, "creating swapchain image!")){                
                    if(result == VK_ERROR_OUT_OF_DATE_KHR){recreate_swapchain(); return;}
                    else if(Log::check_error(result != VK_SUBOPTIMAL_KHR, "suboptimal!")){
                        throw std::runtime_error("failed to aquire swap chain image!");
                        ///TOOOO DOOOO idk what this means anymore
                    }
                }
                VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
                vulkan_component->submit_info = {
                    .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
                    .waitSemaphoreCount = 1,
                    .pWaitSemaphores = &vulkan_component->semaphores.image_available,
                    .pWaitDstStageMask = wait_stages,
                    .commandBufferCount = 1,
                    .pCommandBuffers = &vulkan_component->command.buffers[image_index],
                    .signalSemaphoreCount = 1,
                    .pSignalSemaphores = &vulkan_component->semaphores.render_finished
                };
                Log::check_error(VK_SUCCESS == vkQueueSubmit(vulkan_component->queues.graphics, 1, &vulkan_component->submit_info, VK_NULL_HANDLE), "GRAPHICS QUEUE SUBMIT");
            }

            void Raster::end_frame(const uint32_t &image_index)
            {
                VkPresentInfoKHR present_info = {
                    .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
                    .waitSemaphoreCount = vulkan_component->submit_info.signalSemaphoreCount,
                    .pWaitSemaphores = vulkan_component->submit_info.pSignalSemaphores,
                    .swapchainCount = 1,
                    .pSwapchains = &vulkan_component->swapchain.get,
                    .pImageIndices = &image_index,
                    .pResults = nullptr
                };

                vkQueueWaitIdle(vulkan_component->queues.present);

                VkResult result = vkQueuePresentKHR(vulkan_component->queues.present, &present_info);
                if(!Log::check_error(result == VK_SUCCESS, "creating swapchain image!")){                
                    if(result == VK_ERROR_OUT_OF_DATE_KHR){recreate_swapchain(); return;}
                    else if(Log::check_error(result != VK_SUBOPTIMAL_KHR, "suboptimal!")){
                        throw std::runtime_error("failed to aquire swap chain image!");
                        ///TOOOO DOOOO idk what this means anymore
                    }
                }
            }

            void Raster::add_entity(flecs::entity &e)
            {
            }

            void Raster::remove_entity(flecs::entity &e)
            {
            }

            void Raster::process_entity(flecs::entity &e)
            {
            }

            void Raster::end_update()
            {
            }

            void Raster::create_graphics_pipeline()
            {
                VkPipelineInputAssemblyStateCreateInfo input_assembly_state = vks::initializers::pipelineInputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, 0, VK_FALSE);
                VkPipelineRasterizationStateCreateInfo rasterization_state = vks::initializers::pipelineRasterizationStateCreateInfo(VK_POLYGON_MODE_FILL, VK_CULL_MODE_FRONT_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE,0);
                VkPipelineColorBlendAttachmentState blend_attachment_state = vks::initializers::pipelineColorBlendAttachmentState(0xf,VK_FALSE);
                VkPipelineColorBlendStateCreateInfo color_blend_state = vks::initializers::pipelineColorBlendStateCreateInfo(1,&blend_attachment_state);
                VkPipelineDepthStencilStateCreateInfo depth_stencil_state = vks::initializers::pipelineDepthStencilStateCreateInfo(VK_FALSE, VK_FALSE, VK_COMPARE_OP_LESS_OR_EQUAL);
                VkPipelineViewportStateCreateInfo viewport_state = vks::initializers::pipelineViewportStateCreateInfo(1, 1, 0);
                VkPipelineMultisampleStateCreateInfo multisample_state = vks::initializers::pipelineMultisampleStateCreateInfo(VK_SAMPLE_COUNT_1_BIT,0);
                std::vector<VkDynamicState> dynamic_state_enables = {VK_DYNAMIC_STATE_VIEWPORT,VK_DYNAMIC_STATE_SCISSOR};
                VkPipelineDynamicStateCreateInfo dynamic_state = vks::initializers::pipelineDynamicStateCreateInfo( dynamic_state_enables.data(), dynamic_state_enables.size(), 0);
                VkPipelineLayoutCreateInfo pipeline_layout = vks::initializers::pipelineLayoutCreateInfo(0, 0);
                if(!Log::check(vkCreatePipelineLayout(vulkan_component->device.logical, &pipeline_layout, nullptr, &graphics_pipeline->pipeline_layout) == VK_SUCCESS, "Creating pipeline layout")){
                    throw std::runtime_error("Failed creating pipeline layout");
                }

                auto assets_folder = g_world.get<Resource::Cmp_Directory>()->assets;
                
                /*Shader::init();
                const auto vert_shader_code = Shader::compile_glsl(assets_folder + "Shaders/glsl/triangle.vert", Shader::Type::eVertex);
                const auto frag_shader_code = Shader::compile_glsl(assets_folder + "Shaders/glsl/triangle.frag", Shader::Type::eFragment);
                Shader::finalize();    

                if(!Log::check(vert_shader_code.has_value(), "Compiling Vertex Shader")) throw std::runtime_error("failed to compile vertex shader");
                if(!Log::check(frag_shader_code.has_value(), "Compiling Fragment Shader")) throw std::runtime_error("Failed to compile fragment shader");
                
                auto vert_shader_module = vulkan_component->device.createShaderModule(vert_shader_code.value());
                auto frag_shader_module = vulkan_component->device.createShaderModule(frag_shader_code.value());
                
                */
                auto read_file = [](std::string filename){
                    std::ifstream file(filename, std::ios::ate | std::ios::binary);
                    if (!file.is_open()) {
                        throw std::runtime_error("failed to open file: " + filename);
                    }

                    size_t fileSize = (size_t)file.tellg();
                    std::vector<char> buffer(fileSize);

                    file.seekg(0);
                    file.read(buffer.data(), fileSize);

                    file.close();

                    return buffer;
                };
                auto vert_shader_code = read_file(assets_folder + "Shaders/triangle_vert.spv");
                auto frag_shader_code = read_file(assets_folder + "Shaders/triangle_frag.spv");
                auto vert_shader_module = vulkan_component->device.createShaderModule(vert_shader_code);
                auto frag_shader_module = vulkan_component->device.createShaderModule(frag_shader_code);

                
                VkPipelineShaderStageCreateInfo vert_shader_stage_info = {
                    .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                    .stage = VK_SHADER_STAGE_VERTEX_BIT,
                    .module = vert_shader_module,
                    .pName = "main"
                };

                VkPipelineShaderStageCreateInfo frag_shader_stage_info = {
                    .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                    .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
                    .module = frag_shader_module,
                    .pName = "main"
                };
                
                std::array<VkPipelineShaderStageCreateInfo, 2> shader_stages = { vert_shader_stage_info, frag_shader_stage_info };

                auto binding = Shader::V32::get_binding();
                auto attribute = Shader::V32::get_attribute();

                VkPipelineVertexInputStateCreateInfo empty_input_state = {
                    .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
                    .vertexBindingDescriptionCount = 1,
                    .pVertexBindingDescriptions = &binding,// nullptr,
                    .vertexAttributeDescriptionCount = static_cast<uint32_t>(attribute.size()),
                    .pVertexAttributeDescriptions = attribute.data()
                };
                
                VkGraphicsPipelineCreateInfo pipelineCreateInfo = {
                    .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
                    .stageCount = static_cast<uint32_t>(shader_stages.size()), // Cast size to uint32_t
                    .pStages = shader_stages.data(),
                    .pVertexInputState = &empty_input_state,
                    .pInputAssemblyState = &input_assembly_state,
                    .pViewportState = &viewport_state,
                    .pRasterizationState = &rasterization_state,
                    .pMultisampleState = &multisample_state,
                    .pDepthStencilState = &depth_stencil_state,
                    .pColorBlendState = &color_blend_state,
                    .pDynamicState = &dynamic_state,
                    .layout = graphics_pipeline->pipeline_layout,
                    .renderPass = vulkan_component->pipeline.render_pass,
                    .subpass = 0,
                    .basePipelineHandle = VK_NULL_HANDLE,
                    .basePipelineIndex = -1
                };
                
                VkResult result = vkCreateGraphicsPipelines(
                    vulkan_component->device.logical,
                    vulkan_component->pipeline.cache,
                    1,
                    &pipelineCreateInfo,
                    nullptr,
                    &graphics_pipeline->pipeline
                );

                if(!Log::check(result == VK_SUCCESS, "CREATE GRAPHICS PIPELINE")){
                    throw std::runtime_error("Failed to create Graphics piepline");
                }

                vkDestroyShaderModule(vulkan_component->device.logical, frag_shader_module, nullptr);
                vkDestroyShaderModule(vulkan_component->device.logical, vert_shader_module, nullptr);
            }

            void Raster::create_descriptor_pool()
            {
            }

            void Raster::create_descriptor_sets()
            {
            }

            void Raster::create_descriptor_set_layout()
            {
            }

            void Raster::create_command_buffers(float swap_ratio, int32_t offset_width, int32_t offset_height)
            {
                vulkan_component->command.buffers.resize(vulkan_component->swapchain.frame_buffers.size());
                
                vulkan_component->swapchain.scaled.height = vulkan_component->swapchain.extent.height * swap_ratio;
                vulkan_component->swapchain.scaled.width = vulkan_component->swapchain.extent.width * swap_ratio;
                
                VkCommandBufferAllocateInfo allocInfo = {
                    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
                    .commandPool = vulkan_component->command.pool,
                    .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                    .commandBufferCount = (uint32_t)vulkan_component->command.buffers.size()
                };

                //@COMPUTEHERE be sure to have compute-specific command buffers too
                if (vkAllocateCommandBuffers(vulkan_component->device.logical, &allocInfo, vulkan_component->command.buffers.data()) != VK_SUCCESS) {
                    Log::send(Log::Level::ERROR, "failed to allocate command buffers!");
                    throw std::runtime_error("failed to allocate command buffers!");
                }


                for (size_t i = 0; i < vulkan_component->command.buffers.size(); i++) {
                    VkCommandBufferBeginInfo beginInfo = {};
                    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
                    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT; //The cmdbuf will be rerecorded right after executing it 1s
                    beginInfo.pInheritanceInfo = nullptr; // Optional //only for secondary buffers

                    vkBeginCommandBuffer(vulkan_component->command.buffers[i], &beginInfo);
                    VkRect2D render_area = {
                            .offset = { offset_width, offset_height }, 
                            .extent = vulkan_component->swapchain.scaled
                    };

                    VkRenderPassBeginInfo renderPassInfo = {
                        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
                        .renderPass = vulkan_component->pipeline.render_pass,
                        .framebuffer = vulkan_component->swapchain.frame_buffers[i],
                        .renderArea = render_area                        
                    };
                    
                    std::array<VkClearValue, 2> clearValues = {};
                    clearValues[0].color = { 0.0f, 0.0f, 0.0f, 1.0f }; //derp
                    clearValues[1].depthStencil = { 1.0f, 0 }; //1.0 = farplane, 0.0 = nearplane HELP

                    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size()); //cuz
                    renderPassInfo.pClearValues = clearValues.data(); //duh

                    vkCmdBeginRenderPass(vulkan_component->command.buffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

                    VkViewport viewport = vks::initializers::viewport(vulkan_component->swapchain.extent.width, vulkan_component->swapchain.extent.height, 0.0f, 1.0f);
                    vkCmdSetViewport(vulkan_component->command.buffers[i], 0, 1, &viewport);

                    VkRect2D scissor = vks::initializers::rect2D(vulkan_component->swapchain.extent.width, vulkan_component->swapchain.extent.height, 0, 0);
                    vkCmdSetScissor(vulkan_component->command.buffers[i], 0, 1, &scissor);

                    //vkCmdBindDescriptorSets(vulkan_component->command.buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipeline.pipeline_layout, 0, 1, &graphics_pipeline.descriptor_set, 0, NULL);
                    vkCmdBindPipeline(vulkan_component->command.buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS,graphics_pipeline->pipeline);
                    vkCmdDraw(vulkan_component->command.buffers[i], 3, 1, 0, 0);
                    vkCmdEndRenderPass(vulkan_component->command.buffers[i]);

                    Log::check(VK_SUCCESS == vkEndCommandBuffer(vulkan_component->command.buffers[i]), "END COMMAND BUFFER");
                }
            }

            void Raster::clean_up()
            {
                vkDestroyPipelineLayout(vulkan_component->device.logical, graphics_pipeline->pipeline_layout, nullptr);
                vkDestroyRenderPass(vulkan_component->device.logical, vulkan_component->pipeline.render_pass, nullptr);
            }

            void Raster::clean_up_swapchain()
            {
            }
            void Raster::recreate_swapchain()
            {
                RenderBase::recreate_swapchain();    
                create_descriptor_set_layout();
                create_graphics_pipeline();
                create_command_buffers(1.f, 0, 0);  
            }
            void Raster::toggle_playmode(bool b)
            {
            }
            void Raster::add_material(glm::vec3 diff, float rfl, float rough, float trans, float ri)
            {
            }
            void Raster::update_material(std::string name)
            {
            }
            void Raster::update_camera(Cmp_Camera *c)
            {
            }
            void Raster::update_descriptors()
            {
            }
        }
    }
}
