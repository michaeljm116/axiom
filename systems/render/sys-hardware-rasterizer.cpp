#include "sys-hardware-rasterizer.h"
#include "sys-shader.h"
#include "sys-log.h"
#include <array>
#include "cmp-resource.h"
#include <glm/glm.hpp>

namespace Axiom{
    namespace Render{
        namespace Hardware{
            Raster::Raster(){

            }
            Raster::~Raster(){}

            Raster g_raster = Raster();
            void initialize_raster(){
                g_raster.c_vulkan = g_world.get_ref<Axiom::Render::Cmp_Vulkan>().get();
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
                prepare_buffers();
                create_descriptor_set_layout();
                create_graphics_pipeline();
                
                create_descriptor_pool();
                create_descriptor_sets();
                create_command_buffers(1.f, 0, 0);

            }
            
            auto handle_aquire = [](VkResult result){
                if(!Log::check_error(result == VK_SUCCESS, "creating swapchain image")){
                    if(result == VK_ERROR_OUT_OF_DATE_KHR) 
                        g_raster.recreate_swapchain();
                    else if (Log::check_error(result != VK_SUBOPTIMAL_KHR, "suboptimal!")){
                        throw std::runtime_error("failed to aquire swapchain image!");
                    }
                }
                /*if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) {
                    framebufferResized = false;
                    recreateSwapChain();
                } else if (result != VK_SUCCESS) {
                    throw std::runtime_error("failed to present swap chain image!");
                }*/
            };
            void Raster::draw_frame() {
                vkWaitForFences(c_vulkan->device.logical, 1, &c_vulkan->semaphores.presentation_fence[current_frame], VK_TRUE, UINT64_MAX);

                uint32_t imageIndex;
                VkResult result = vkAcquireNextImageKHR(c_vulkan->device.logical, c_vulkan->swapchain.get, UINT64_MAX, c_vulkan->semaphores.render_ready[current_frame], VK_NULL_HANDLE, &imageIndex);
                handle_aquire(result);
                
                update_uniform_buffer(current_frame);

                vkResetFences(c_vulkan->device.logical, 1, & c_vulkan->semaphores.presentation_fence[current_frame]);
                vkResetCommandBuffer(c_vulkan->command.buffers[current_frame], 0 );
                
                
                update_command_buffer(c_vulkan->command.buffers[current_frame], imageIndex);

                VkSubmitInfo submitInfo{};
                submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

                VkSemaphore waitSemaphores[] = {c_vulkan->semaphores.render_ready[current_frame]};
                VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
                submitInfo.waitSemaphoreCount = 1;
                submitInfo.pWaitSemaphores = waitSemaphores;
                submitInfo.pWaitDstStageMask = waitStages;

                submitInfo.commandBufferCount = 1;
                submitInfo.pCommandBuffers = &c_vulkan->command.buffers[current_frame];

                VkSemaphore signalSemaphores[] = {c_vulkan->semaphores.frame_presented[current_frame]};
                submitInfo.signalSemaphoreCount = 1;
                submitInfo.pSignalSemaphores = signalSemaphores;

                if (vkQueueSubmit(c_vulkan->queues.graphics, 1, &submitInfo, c_vulkan->semaphores.presentation_fence[current_frame]) != VK_SUCCESS) {
                    throw std::runtime_error("failed to submit draw command buffer!");
                }

                VkPresentInfoKHR presentInfo{};
                presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

                presentInfo.waitSemaphoreCount = 1;
                presentInfo.pWaitSemaphores = signalSemaphores;

                VkSwapchainKHR swapChains[] = {c_vulkan->swapchain.get};
                presentInfo.swapchainCount = 1;
                presentInfo.pSwapchains = swapChains;

                presentInfo.pImageIndices = &imageIndex;

                result = vkQueuePresentKHR(c_vulkan->queues.present, &presentInfo);
                handle_aquire(result);

                current_frame = (current_frame + 1) % MAX_FRAMES_IN_FLIGHT;
            }
            void Raster::start_frame(uint32_t &image_index)
            {
                //update_uniform_buffer();

                auto result = vkAcquireNextImageKHR(c_vulkan->device.logical, c_vulkan->swapchain.get,
                                                    std::numeric_limits<uint64_t>::max(), 
                                                    c_vulkan->semaphores.image_available, VK_NULL_HANDLE, &image_index);
                handle_aquire(result);

                VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
                c_vulkan->submit_info = {
                    .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
                    .waitSemaphoreCount = 1,
                    .pWaitSemaphores = &c_vulkan->semaphores.image_available,
                    .pWaitDstStageMask = wait_stages,
                    .commandBufferCount = 1,
                    .pCommandBuffers = &c_vulkan->command.buffers[image_index],
                    .signalSemaphoreCount = 1,
                    .pSignalSemaphores = &c_vulkan->semaphores.render_finished
                };
                Log::check_error(VK_SUCCESS == vkQueueSubmit(c_vulkan->queues.graphics, 1, &c_vulkan->submit_info, VK_NULL_HANDLE), "GRAPHICS QUEUE SUBMIT");
            }

            void Raster::end_frame(const uint32_t &image_index)
            {
                VkPresentInfoKHR present_info = {
                    .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
                    .waitSemaphoreCount = c_vulkan->submit_info.signalSemaphoreCount,
                    .pWaitSemaphores = c_vulkan->submit_info.pSignalSemaphores,
                    .swapchainCount = 1,
                    .pSwapchains = &c_vulkan->swapchain.get,
                    .pImageIndices = &image_index,
                    .pResults = nullptr
                };

                vkQueueWaitIdle(c_vulkan->queues.present);

                VkResult result = vkQueuePresentKHR(c_vulkan->queues.present, &present_info);
                handle_aquire(result);

                current_frame = (current_frame + 1) % MAX_FRAMES_IN_FLIGHT;
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
                VkPipelineLayoutCreateInfo pipeline_layout = vks::initializers::pipelineLayoutCreateInfo(&graphics_pipeline->descriptor_set_layout, 1);
                if(!Log::check(vkCreatePipelineLayout(c_vulkan->device.logical, &pipeline_layout, nullptr, &graphics_pipeline->pipeline_layout) == VK_SUCCESS, "Creating pipeline layout")){
                    throw std::runtime_error("Failed creating pipeline layout");
                }

                auto assets_folder = g_world.get<Resource::Cmp_Directory>()->assets;
                
                /*Shader::init();
                const auto vert_shader_code = Shader::compile_glsl(assets_folder + "Shaders/glsl/triangle.vert", Shader::Type::eVertex);
                const auto frag_shader_code = Shader::compile_glsl(assets_folder + "Shaders/glsl/triangle.frag", Shader::Type::eFragment);
                Shader::finalize();    

                if(!Log::check(vert_shader_code.has_value(), "Compiling Vertex Shader")) throw std::runtime_error("failed to compile vertex shader");
                if(!Log::check(frag_shader_code.has_value(), "Compiling Fragment Shader")) throw std::runtime_error("Failed to compile fragment shader");
                
                auto vert_shader_module = c_vulkan->device.createShaderModule(vert_shader_code.value());
                auto frag_shader_module = c_vulkan->device.createShaderModule(frag_shader_code.value());
                
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
                auto vert_shader_code = read_file(assets_folder + "Shaders/basic_vert.spv");
                auto frag_shader_code = read_file(assets_folder + "Shaders/basic_frag.spv");
                auto vert_shader_module = c_vulkan->device.createShaderModule(vert_shader_code);
                auto frag_shader_module = c_vulkan->device.createShaderModule(frag_shader_code);

                
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
                    .renderPass = c_vulkan->pipeline.render_pass,
                    .subpass = 0,
                    .basePipelineHandle = VK_NULL_HANDLE,
                    .basePipelineIndex = -1
                };
                
                VkResult result = vkCreateGraphicsPipelines(
                    c_vulkan->device.logical,
                    c_vulkan->pipeline.cache,
                    1,
                    &pipelineCreateInfo,
                    nullptr,
                    &graphics_pipeline->pipeline
                );

                if(!Log::check(result == VK_SUCCESS, "CREATE GRAPHICS PIPELINE")){
                    throw std::runtime_error("Failed to create Graphics piepline");
                }

                vkDestroyShaderModule(c_vulkan->device.logical, frag_shader_module, nullptr);
                vkDestroyShaderModule(c_vulkan->device.logical, vert_shader_module, nullptr);
            }

            void Raster::create_descriptor_pool()
            {
                std::array<VkDescriptorPoolSize, 1> pool_sizes = {vks::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, MAX_FRAMES_IN_FLIGHT)};
                auto create_info = vks::initializers::descriptorPoolCreateInfo(pool_sizes.size(), pool_sizes.data(), MAX_FRAMES_IN_FLIGHT);
                if(!Log::check_error(vkCreateDescriptorPool(c_vulkan->device.logical, & create_info, nullptr, &graphics_pipeline->descriptor_pool) == VK_SUCCESS, "creating descriptor pool"))
                    throw std::runtime_error("Error creating descriptor pool");
            }

            void Raster::create_descriptor_sets()
            {
                std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, graphics_pipeline->descriptor_set_layout);
                graphics_pipeline->descriptor_sets.resize(MAX_FRAMES_IN_FLIGHT);
                auto alloc_info = vks::initializers::descriptorSetAllocateInfo(graphics_pipeline->descriptor_pool, layouts.data(), MAX_FRAMES_IN_FLIGHT);// MAX_FRAMES_IN_FLIGHT);
                
                if(!Log::check_error(vkAllocateDescriptorSets(c_vulkan->device.logical, &alloc_info, graphics_pipeline->descriptor_sets.data()) == VK_SUCCESS, "ALLOCATING descriptor sets"))
                    throw std::runtime_error("Error allocating descriptorset");
                
                for(int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i){
                    auto ds_info = VkDescriptorBufferInfo{
                        .buffer = uniform_buffers[i].buffer,
                        .offset = 0,
                        .range = sizeof(ubo)
                    };

                    auto write_ds = vks::initializers::writeDescriptorSet(
                        graphics_pipeline->descriptor_sets[i], 
                        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                        0, &ds_info, 1
                    );

                    vkUpdateDescriptorSets(c_vulkan->device.logical, 1, &write_ds, 0, nullptr);
                }

            }

            void Raster::create_descriptor_set_layout()
            {
                VkDescriptorSetLayoutBinding ubo_layout_binding = {
                    .binding = 0, .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, .descriptorCount = 1,
                    .stageFlags = VK_SHADER_STAGE_VERTEX_BIT, .pImmutableSamplers = nullptr
                };
                VkDescriptorSetLayoutCreateInfo ds_info = {
                    .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
                    .pNext = nullptr, .bindingCount = 1, .pBindings = &ubo_layout_binding
                };
                if(!Log::check_error(vkCreateDescriptorSetLayout(c_vulkan->device.logical, &ds_info, nullptr, &graphics_pipeline->descriptor_set_layout) == VK_SUCCESS, "Create descriptor set layout!"))
                    throw std::runtime_error("failed to create descriptor set layout!");
            }

            void Raster::create_command_buffers(float swap_ratio, int32_t offset_width, int32_t offset_height)
            {
                c_vulkan->command.buffers.resize(MAX_FRAMES_IN_FLIGHT);
                
                c_vulkan->swapchain.scaled.height = c_vulkan->swapchain.extent.height * swap_ratio;
                c_vulkan->swapchain.scaled.width = c_vulkan->swapchain.extent.width * swap_ratio;
                
                VkCommandBufferAllocateInfo allocInfo = {
                    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
                    .commandPool = c_vulkan->command.pool,
                    .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                    .commandBufferCount = (uint32_t)c_vulkan->command.buffers.size()
                };

                //@COMPUTEHERE be sure to have compute-specific command buffers too
                if (vkAllocateCommandBuffers(c_vulkan->device.logical, &allocInfo, c_vulkan->command.buffers.data()) != VK_SUCCESS) {
                    Log::send(Log::Level::ERROR, "failed to allocate command buffers!");
                    throw std::runtime_error("failed to allocate command buffers!");
                }
                /*
                for (size_t i = 0; i < c_vulkan->command.buffers.size(); i++) 
                {
                    VkCommandBufferBeginInfo beginInfo = {};
                    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
                    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT; //The cmdbuf will be rerecorded right after executing it 1s
                    beginInfo.pInheritanceInfo = nullptr; // Optional //only for secondary buffers

                    vkBeginCommandBuffer(c_vulkan->command.buffers[i], &beginInfo);
                    VkRect2D render_area = {
                            .offset = { offset_width, offset_height }, 
                            .extent = c_vulkan->swapchain.scaled
                    };

                    VkRenderPassBeginInfo renderPassInfo = {
                        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
                        .renderPass = c_vulkan->pipeline.render_pass,
                        .framebuffer = c_vulkan->swapchain.frame_buffers[i],
                        .renderArea = render_area                        
                    };
                    
                    std::array<VkClearValue, 2> clearValues = {};
                    clearValues[0].color = { 0.0f, 0.0f, 0.0f, 1.0f }; //derp
                    clearValues[1].depthStencil = { 1.0f, 0 }; //1.0 = farplane, 0.0 = nearplane HELP

                    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size()); //cuz
                    renderPassInfo.pClearValues = clearValues.data(); //duh

                    vkCmdBeginRenderPass(c_vulkan->command.buffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

                    VkViewport viewport = vks::initializers::viewport(c_vulkan->swapchain.extent.width, c_vulkan->swapchain.extent.height, 0.0f, 1.0f);
                    vkCmdSetViewport(c_vulkan->command.buffers[i], 0, 1, &viewport);

                    VkRect2D scissor = vks::initializers::rect2D(c_vulkan->swapchain.extent.width, c_vulkan->swapchain.extent.height, 0, 0);
                    vkCmdSetScissor(c_vulkan->command.buffers[i], 0, 1, &scissor);

                    
                    VkBuffer vertexBuffers[] = {vertex_buffer.buffer};
                    VkDeviceSize offsets[] = {0};
                    vkCmdBindVertexBuffers(c_vulkan->command.buffers[i], 0, 1, vertexBuffers, offsets);
                    vkCmdBindIndexBuffer(c_vulkan->command.buffers[i], index_buffer.buffer, 0, VK_INDEX_TYPE_UINT32);
                    vkCmdBindDescriptorSets(c_vulkan->command.buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipeline->pipeline_layout, 0, 1, &graphics_pipeline->descriptor_sets[current_frame], 0, nullptr);
                    //vkCmdBindPipeline(c_vulkan->command.buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS,graphics_pipeline->pipeline);
                    vkCmdDrawIndexed(c_vulkan->command.buffers[i], static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
                    vkCmdEndRenderPass(c_vulkan->command.buffers[i]);

                    Log::check(VK_SUCCESS == vkEndCommandBuffer(c_vulkan->command.buffers[i]), "END COMMAND BUFFER");
                }*/
            }

            void Raster::update_command_buffer(VkCommandBuffer command_buffer, uint32_t image_index)
            {
                    VkCommandBufferBeginInfo beginInfo = {};
                    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
                    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT; //The cmdbuf will be rerecorded right after executing it 1s
                    beginInfo.pInheritanceInfo = nullptr; // Optional //only for secondary buffers

                    vkBeginCommandBuffer(command_buffer, &beginInfo);
                    VkRect2D render_area = {
                            .offset = { 0, 0 }, 
                            .extent = c_vulkan->swapchain.scaled
                    };

                    VkRenderPassBeginInfo renderPassInfo = {
                        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
                        .renderPass = c_vulkan->pipeline.render_pass,
                        .framebuffer = c_vulkan->swapchain.frame_buffers[image_index],
                        .renderArea = render_area                        
                    };
                    
                    std::array<VkClearValue, 2> clearValues = {};
                    clearValues[0].color = { 0.0f, 0.0f, 0.0f, 1.0f }; //derp
                    clearValues[1].depthStencil = { 1.0f, 0 }; //1.0 = farplane, 0.0 = nearplane HELP

                    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size()); //cuz
                    renderPassInfo.pClearValues = clearValues.data(); //duh

                    vkCmdBeginRenderPass(command_buffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
                    {
                        vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipeline->pipeline);
                        VkViewport viewport = vks::initializers::viewport(c_vulkan->swapchain.extent.width, c_vulkan->swapchain.extent.height, 0.0f, 1.0f);
                        vkCmdSetViewport(command_buffer, 0, 1, &viewport);

                        VkRect2D scissor = vks::initializers::rect2D(c_vulkan->swapchain.extent.width, c_vulkan->swapchain.extent.height, 0, 0);
                        vkCmdSetScissor(command_buffer, 0, 1, &scissor);

                        
                        VkBuffer vertexBuffers[] = {vertex_buffer.buffer};
                        VkDeviceSize offsets[] = {0};
                        vkCmdBindVertexBuffers(command_buffer, 0, 1, vertexBuffers, offsets);
                        vkCmdBindIndexBuffer(command_buffer, index_buffer.buffer, 0, VK_INDEX_TYPE_UINT32);
                        vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipeline->pipeline_layout, 0, 1, &graphics_pipeline->descriptor_sets[current_frame], 0, nullptr);
                        
                        vkCmdDrawIndexed(command_buffer, static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
                    }
                    vkCmdEndRenderPass(command_buffer);

                    Log::check_error(VK_SUCCESS == vkEndCommandBuffer(command_buffer), "END COMMAND BUFFER");                
            }

            void Raster::update_uniform_buffer(uint32_t current_frame)
            {
                static auto start_time = std::chrono::high_resolution_clock::now();
                auto curr_time = std::chrono::high_resolution_clock::now();
                float time = std::chrono::duration<float, std::chrono::seconds::period>(curr_time - start_time).count();
                
                ubo.model = glm::rotate(glm::mat4(1.f), time * glm::radians(90.f), glm::vec3(0.f, 0.f, 1.f));
                ubo.view = glm::lookAt(glm::vec3(2.f, 2.f, 2.f), glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 0.f, 1.f));
                ubo.proj = glm::perspective(glm::radians(45.f), c_vulkan->swapchain.extent.width / (float) c_vulkan->swapchain.extent.height, 0.1f, 10.0f);
                //ubo.proj[1][1] *= -1;

                uniform_buffers[current_frame].ApplyChanges(c_vulkan->device, ubo);

            }

            void Raster::prepare_buffers()
            {
                vertex_buffer.InitStorageBufferCustomSize(c_vulkan->device, vertices, vertices.size(), vertices.size());
                index_buffer.InitStorageBufferCustomSize(c_vulkan->device, indices, indices.size(), indices.size());
                for(int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
                uniform_buffers[i].InitUniformBuffer(c_vulkan->device, ubo);
            }

            void Raster::clean_up()
            {
                vkDestroyPipelineLayout(c_vulkan->device.logical, graphics_pipeline->pipeline_layout, nullptr);
                vkDestroyRenderPass(c_vulkan->device.logical, c_vulkan->pipeline.render_pass, nullptr);
            }

            void Raster::clean_up_swapchain()
            {
                vertex_buffer.Destroy(c_vulkan->device);
                index_buffer.Destroy(c_vulkan->device);
                for(int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
                    uniform_buffers[i].Destroy(c_vulkan->device);
                

                vkDestroyPipeline(c_vulkan->device.logical, graphics_pipeline->pipeline, nullptr);
                vkDestroyDescriptorSetLayout(c_vulkan->device.logical, graphics_pipeline->descriptor_set_layout, nullptr);
                RenderBase::clean_up_swapchain();

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
