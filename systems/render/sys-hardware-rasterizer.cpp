#include "sys-hardware-rasterizer.h"
#include "sys-shader.h"
#include "sys-log.h"
#include <array>


namespace Axiom{
    namespace Render{
        namespace Hardware{

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

                Shader::init();
                const auto vert_shader_code = Shader::compile_glsl("../Assets/Shaders/glsl/triangle_vert.glsl", Shader::Type::eVertex);
                const auto frag_shader_code = Shader::compile_glsl("../Assets/Shaders/glsl/triangle_frag.glsl", Shader::Type::eFragment);
                Shader::finalize();
                
                auto vert_shader_module = vulkan_component->device.createShaderModule(vert_shader_code.value());
                auto frag_shader_module = vulkan_component->device.createShaderModule(frag_shader_code.value());

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


            // Assuming vertShaderStageInfo and fragShaderStageInfo are already defined
                std::array<VkPipelineShaderStageCreateInfo, 2> shader_stages = { vert_shader_stage_info, frag_shader_stage_info };

                // Initialize VkPipelineVertexInputStateCreateInfo using designated initializers
                VkPipelineVertexInputStateCreateInfo empty_input_state = {
                    .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
                    .vertexBindingDescriptionCount = 0,
                    .pVertexBindingDescriptions = nullptr,
                    .vertexAttributeDescriptionCount = 0,
                    .pVertexAttributeDescriptions = nullptr
                };

                // Assuming inputAssemblyState, rasterizationState, colorBlendState, multisampleState,
                // viewportState, depthStencilState, and dynamicState are already defined and initialized
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
                    .layout = graphics_pipeline.pipeline_layout,
                    .renderPass = vulkan_component->pipeline.render_pass,
                    .subpass = 0,
                    .basePipelineHandle = VK_NULL_HANDLE,
                    .basePipelineIndex = -1
                };

                // Creation of the graphics pipeline
                VkResult result = vkCreateGraphicsPipelines(
                    vulkan_component->device.logical,
                    vulkan_component->pipeline.cache,
                    1, // Pipeline count
                    &pipelineCreateInfo,
                    nullptr, // Allocator
                    &graphics_pipeline.pipeline
                );

                // Using a helper function for logging the result
                Log::check(result == VK_SUCCESS, "CREATE GRAPHICS PIPELINE");

                
                vkDestroyShaderModule(vulkan_component->device.logical, frag_shader_module, nullptr);
                vkDestroyShaderModule(vulkan_component->device.logical, vert_shader_module, nullptr);
            }

            void Raster::clean_up()
            {
                vkDestroyPipelineLayout(vulkan_component->device.logical, graphics_pipeline.pipeline_layout, nullptr);
                vkDestroyRenderPass(vulkan_component->device.logical, vulkan_component->pipeline.render_pass, nullptr);
            }

        }
    }
}
