#include "pch.h"
#include "sys-compute-raytracer.h"
#include "core/sys-log.h"
#include "core/sys-timer.h"
#include "../components/scene/cmp-transform.h"
#include "../components/render/cmp-material.h"

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
                this->world = world;
                initVulkan();
                set_stuff_up();
                std::vector<rMaterial> copy = RESOURCEMANAGER.getMaterials();
                for (std::vector<rMaterial>::iterator itr = copy.begin(); itr != copy.end(); ++itr) {
                    compute_component->shader_data.materials.push_back(ssMaterial(itr->diffuse, itr->reflective, itr->roughness, itr->transparency, itr->refractiveIndex, itr->textureID));
                    itr->renderedMat = &compute_component->shader_data.materials.back();// compute_component->shader_data.materials.end();
                }
                LoadResources();
            }
            void Raytracer::initialize()
            {
                mapper_ = render_mapper;
                //renderMapper.init(*world);
                prepare_storage_buffers();
                create_uniform_buffers();
                prepare_texture_target(&compute_texture_, 1280, 720, VK_FORMAT_R8G8B8A8_UNORM);
                create_descriptor_set_layout();
                create_graphics_pipeline();
                create_descriptor_pool();
                create_descriptor_sets();
                prepare_compute();
#ifdef UIIZON
                create_command_buffers(0.733333333333f, (int32_t)(WINDOW.getWidth() * 0.16666666666f), 36);
#else
                create_command_buffers(1.f, 0, 0);
#endif // UIIZON

                update_descriptors();

                //setupUI();
                prepared_ = true;

            }
            void Raytracer::start_frame(uint32_t &image_index)
            {
                //render_time_.Start();
                VkResult result = vkAcquireNextImageKHR(vulkan_component->device.logical, vulkan_component->swapchain.get, std::numeric_limits<uint64_t>::max(), imageAvailableSemaphore, VK_NULL_HANDLE, &image_index);

                if (result == VK_ERROR_OUT_OF_DATE_KHR) {
                    recreate_swapchain();
                    return;
                }
                else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
                    throw std::runtime_error("failed to acquire swap chain image!");
                }

                vulkan_component->submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
                vulkan_component->submit_info.commandBufferCount = 1;
                vulkan_component->submit_info.pCommandBuffers = &vulkan_component->command.buffers[image_index];

                VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
                vulkan_component->submit_info.waitSemaphoreCount = 1;
                vulkan_component->submit_info.pWaitSemaphores = &imageAvailableSemaphore;// waitSemaphores;
                vulkan_component->submit_info.pWaitDstStageMask = waitStages;

                vulkan_component->submit_info.signalSemaphoreCount = 1;
                vulkan_component->submit_info.pSignalSemaphores = &vulkan_component->semaphores.render_finished;

                Log::check(VK_SUCCESS == vkQueueSubmit(vulkan_component->queues.graphics, 1, &vulkan_component->submit_info, VK_NULL_HANDLE), "GRAPHICS QUEUE SUBMIT");
            }
            void Raytracer::end_frame(const uint32_t &image_index)
            {
                VkPresentInfoKHR presentInfo = {};
                presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
                presentInfo.waitSemaphoreCount = vulkan_component->submit_info.signalSemaphoreCount;
                presentInfo.pWaitSemaphores = vulkan_component->submit_info.pSignalSemaphores;//ui->visible ? &uiSemaphore : &renderFinishedSemaphore;// signalSemaphores;
                presentInfo.pResults = nullptr; //optional

                VkSwapchainKHR swapChains[] = { vulkan_component->swapchain.get };
                presentInfo.swapchainCount = 1;
                presentInfo.pSwapchains = swapChains;
                presentInfo.pImageIndices = &image_index;

                VkResult result = vkQueuePresentKHR(vulkan_component->queues.present, &presentInfo);
                if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
                    recreate_swapchain();
                }
                else if (result != VK_SUCCESS) {
                    throw std::runtime_error("failed to present swap chain image!");
                }
                vkQueueWaitIdle(vulkan_component->queues.present);// sync with gpu

                //Possible compute here? image is in swapchain so maybe you can use image for compute stuff...
                vkWaitForFences(vulkan_component->device.logical, 1, &raytracing_component->compute.fence, VK_TRUE, UINT64_MAX);
                vkResetFences(vulkan_component->device.logical, 1, &raytracing_component->compute.fence);

                VkSubmitInfo compute_submit_info = {};
                compute_submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
                compute_submit_info.commandBufferCount = 1;
                compute_submit_info.pCommandBuffers = &raytracing_component->compute.command_buffer;// &computeCommandBuffer;

                if (vkQueueSubmit(vulkan_component->queues.compute, 1, &compute_submit_info, raytracing_component->compute.fence) != VK_SUCCESS)
                    throw std::runtime_error("failed to submit compute commadn buffer!");
                //render_time_.End();
                //pINPUT.renderTime = render_time_.ms;
            }
            void Raytracer::add_entity(flecs::entity &e)
            {
                auto* render_component = e.get_mut<Cmp_Render>();
                RenderType t = render_component->type;

                if (t == RENDER_MATERIAL) {

                }
                if (t == RENDER_PRIMITIVE) {
                    auto* primitive_component = e.get_mut<Cmp_Primitive>();
                    auto* material_component = e.get_mut<Cmp_Material>();
                    auto* transform_component = e.get_mut<Cmp_Transform>();

                    primitive_component->matId = material_component->id;
                    primitive_component->world = transform_component->world;
                    primitive_component->extents = transform_component->local.sca;
                    if (primitive_component->id > 0) {
                        std::pair<int, int> temp = compute_component->mesh_assigner[primitive_component->id];
                        primitive_component->startIndex = temp.first;
                        primitive_component->endIndex = temp.second;
                    }
                    SetRenderUpdate(RenderUpdate::kUpdateObject);
                }
                if (t == RENDER_LIGHT) {

                    LightComponent* lightComp = (LightComponent*)e.getComponent<LightComponent>();
                    TransformComponent* transComp = (TransformComponent*)e.getComponent<TransformComponent>();
                    ssLight light;
                    light.pos = transComp->global.position;
                    light.color = lightComp->color;
                    light.intensity = lightComp->intensity;
                    light.id = e.getUniqueId();// lightComp->id;
                    lightComp->id = light.id;
                    lights_.push_back(light);
                    light_comps_.push_back(lightComp);

                    //NodeComponent* node = (NodeComponent*)e.getComponent<NodeComponent>();
                    //addNode(node);

                    compute_component->storage_buffers.lights.UpdateAndExpandBuffers(vulkan_component->device, lights_, lights_.size());
                    //update_descriptors();
                }
                if (t == RENDER_GUI) {
                    GUIComponent* gc = (GUIComponent*)e.getComponent<GUIComponent>();
                    ssGUI gui = ssGUI(gc->min, gc->extents, gc->alignMin, gc->alignExt, gc->layer, gc->id);
                    gc->ref = guis_.size();
                    gui.alpha = gc->alpha;
                    guis_.push_back(gui);
                    SetRenderUpdate(kUpdateGui);
                }
                if (t == RENDER_GUINUM) {
                    GUINumberComponent* gnc = (GUINumberComponent*)e.getComponent<GUINumberComponent>();
                    std::vector<int> nums = intToArrayOfInts(gnc->number);
                    for (int i = 0; i < nums.size(); ++i) {
                        ssGUI gui = ssGUI(gnc->min, gnc->extents, glm::vec2(0.1f * nums[i], 0.f), glm::vec2(0.1f, 1.f), 0, 0);
                        gnc->shaderReferences.push_back(guis_.size());
                        gui.alpha = gnc->alpha;
                        guis_.push_back(gui);
                    }
                    gnc->ref = gnc->shaderReferences[0];
                    SetRenderUpdate(kUpdateGui);
                }
                if (t == RENDER_CAMERA) {
                    CameraComponent* cam = (CameraComponent*)e.getComponent<CameraComponent>();
                    TransformComponent* transComp = (TransformComponent*)e.getComponent<TransformComponent>();
                    raytracing_component->compute.ubo.aspect_ratio = cam->aspectRatio;
                    raytracing_component->compute.ubo.rotM = transComp->world;
                    raytracing_component->compute.ubo.fov = cam->fov;
                }
            }
            void Raytracer::remove_entity(flecs::entity &e)
            {
                RenderType t = ((RenderComponent*)e.getComponent<RenderComponent>())->type;// renderMapper.get(e)->type;

                if (t == RENDER_LIGHT) {
                    if (lights_.size() == 1) {
                        lights_.clear();
                        light_comps_.clear();
                    }
                    else {
                        auto* lc = (LightComponent*)e.getComponent<LightComponent>();
                        for (auto it = lights_.begin(); it != lights_.end(); ++it) {
                            if (lc->id == it->id)
                                lights_.erase(it);
                        }
                    }
                }
                /*else if (t == RENDER_GUINUM) {
                    auto* gnc = (GUINumberComponent*)e.getComponent<GUINumberComponent>();
                    //gnc->

                }*/
            }
            void Raytracer::process_entity(flecs::entity &e)
            {
                RenderType type = ((RenderComponent*)e.getComponent<RenderComponent>())->type;// renderMapper.get(e)->type;
                if (type == RENDER_NONE) return;
                switch (type)
                {
                case RENDER_MATERIAL:
                    SetRenderUpdate(RenderUpdate::kUpdateMaterial);
                    break;
                case RENDER_PRIMITIVE:
                    SetRenderUpdate(RenderUpdate::kUpdateObject);
                    break;
                case RENDER_LIGHT:
                    SetRenderUpdate(RenderUpdate::kUpdateLight);
                    break;
                case RENDER_GUI: {
                    GUIComponent* gui = (GUIComponent*)e.getComponent<GUIComponent>();
                    UpdateGui(gui);
                    break; }
                case RENDER_GUINUM: {
                    GUINumberComponent* gnc = (GUINumberComponent*)e.getComponent<GUINumberComponent>();			
                    //if (gnc->number > 9) {
                    //	/*auto* nodular = (NodeComponent*)e.getComponent<NodeComponent>();
                    //	std::cout << nodular->name + ": " << gnc->number;*/
                    //}
                    if (gnc->update) {
                        gnc->update = false;
                        UpdateGuiNumber(gnc);
                    }
                    break; }
                default:
                    break;
                }
                type = RENDER_NONE;
            }
            void Raytracer::end_update()
            {
                update_buffers();
                update_descriptors();
                if (glfwWindowShouldClose(WINDOW.getWindow())) {
                    world->setShutdown();
                    vulkan_component->deviceWaitIdle(vulkan_component->device.logical); //so it can destroy properly
                }
            }
            void Raytracer::clean_up_swapchain()
            {
                vkDestroyPipeline(vulkan_component->device.logical, raytracing_component->graphics.pipeline, nullptr);
                //vkDestroyPipeline(vulkan_component->device.logical, raytracing_component->graphics.raster.pipeline, nullptr);
                vkDestroyPipelineLayout(vulkan_component->device.logical, raytracing_component->graphics.pipeline_layout, nullptr);
                //vkDestroyPipelineLayout(vulkan_component->device.logical, raytracing_component->graphics.raster.pipeline_layout, nullptr);

                RenderBase::clean_up_swapchain();
            }
            void Raytracer::recreate_swapchain()
            {
                RenderBase::recreate_swapchain();
                create_descriptor_set_layout();
                create_graphics_pipeline();
                //editor ?
                create_command_buffers(0.7333333333f, (int32_t)(WINDOW.getWidth() * 0.16666666666f), 36);
                //	create_command_buffers(0.6666666666666f, 0, 0);
                vulkan_component->swapchain.frame_buffers;
                //return vulkan_component->swapchain.frame_buffers;
                //ui->visible = false;
                //ui->resize(swapChainExtent.width, swapChainExtent.height, vulkan_component->swapchain.frame_buffers);
            }
            void Raytracer::toggle_playmode(bool b)
            {
                if (play_mode) {
                    WINDOW.resize();
                    RenderBase::recreate_swapchain();
                    create_descriptor_set_layout();
                    create_graphics_pipeline();
                    create_command_buffers(1.f, 0, 0);
        //#ifdef UIIZON
        //			create_command_buffers(0.733333333333f, (int32_t)(WINDOW.getWidth() * 0.16666666666f), 36);
        //#else
        //			create_command_buffers(1.f, 0, 0);
        //#endif // UIIZON

                    //ui->resize(swapChainExtent.width, swapChainExtent.height, vulkan_component->swapchain.frame_buffers);
                }
                else {
                    recreate_swapchain();
                }
            }
            void Raytracer::add_material(glm::vec3 diff, float rfl, float rough, float trans, float ri)
            {
                ssMaterial mat = ssMaterial(diff, rfl, rough, trans, ri, 0);
                compute_component->shader_data.materials.push_back(mat);
                compute_component->storage_buffers.materials.UpdateAndExpandBuffers(vulkan_component->device, materials_, compute_component->shader_data.materials.size());
                update_descriptors();
            }
            void Raytracer::update_material(int id)
            {
                rMaterial* m = &RESOURCEMANAGER.getMaterial(id);
                materials_[id].diffuse = m->diffuse;
                materials_[id].reflective = m->reflective;
                materials_[id].roughness = m->roughness;
                materials_[id].transparency = m->transparency;
                materials_[id].refractiveIndex = m->refractiveIndex;
                materials_[id].textureID = m->textureID;

                compute_component->storage_buffers.materials.UpdateBuffers(vulkan_component->device, materials_);
            }
            void Raytracer::update_camera(Cmp_Camera* c){
                compute_.ubo.aspect_ratio = c->aspectRatio;
                compute_.ubo.fov = glm::tan(c->fov * 0.03490658503);
                compute_.ubo.rotM = c->rotM;
                compute_.ubo.rand = random_int();
                compute_.uniform_buffer.ApplyChanges(vkDevice, compute_.ubo);          
            }
            void Raytracer::update_descriptors()
            {
                vkWaitForFences(vulkan_component->device.logical, 1, &raytracing_component->compute.fence, VK_TRUE, UINT64_MAX);
                compute_write_descriptor_sets_ =
                {
                    // Binding 5: for objects
                    vks::initializers::writeDescriptorSet(
                        raytracing_component->compute.descriptor_set,
                        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                        6,
                        &compute_component->storage_buffers.primitives.bufferInfo),
                    //Binding 6 for materials
                    vks::initializers::writeDescriptorSet(
                        raytracing_component->compute.descriptor_set,
                        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                        7,
                        &compute_component->storage_buffers.materials.bufferInfo),
                    //Binding 7 for lights
                    vks::initializers::writeDescriptorSet(
                        raytracing_component->compute.descriptor_set,
                        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                        8,
                        &compute_component->storage_buffers.lights.bufferInfo),
                    //Binding 8 for gui
                    vks::initializers::writeDescriptorSet(
                        raytracing_component->compute.descriptor_set,
                        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                        9,
                        &compute_component->storage_buffers.guis.bufferInfo),
                    //Binding 10 for bvhnodes
                    vks::initializers::writeDescriptorSet(
                        raytracing_component->compute.descriptor_set,
                        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                        10,
                        &compute_component->storage_buffers.bvh.bufferInfo)
                };
                vkupdate_descriptorsets(vulkan_component->device.logical, compute_write_descriptor_sets_.size(), compute_write_descriptor_sets_.data(), 0, NULL);
                //vkupdate_descriptorsets(vulkan_component->device.logical, compute_write_descriptor_sets_.size(), compute_write_descriptor_sets_.data(), 0, NULL);
                CreateComputeCommandBuffer();
            }
            void Raytracer::update_buffers(){
                vkWaitForFences(vkDevice.logicalDevice, 1, &compute_.fence, VK_TRUE, UINT64_MAX);
                if (update_flags_ & kUpdateNone)
                    return;
                if (update_flags_ & kUpdateObject) {
                    //compute_.storage_buffers.primitives.UpdateBuffers(vkDevice, primitives);
                    update_flags_ &= ~kUpdateObject;
                }
                if (update_flags_ & kUpdateMaterial) {
                    update_flags_ &= ~kUpdateMaterial;
                }
                if (update_flags_ & kUpdateLight) {
                    compute_.storage_buffers.lights.UpdateBuffers(vkDevice, lights_);
                    update_flags_ &= ~kUpdateLight;
                }
                if (update_flags_ & kUpdateGui) {
                    compute_.storage_buffers.guis.UpdateBuffers(vkDevice, guis_);
                    update_flags_ &= ~kUpdateGui;
                }

                if (update_flags_ & kUpdateBvh) {
                    compute_.storage_buffers.primitives.UpdateAndExpandBuffers(vkDevice, primitives_, primitives_.size());
                    compute_.storage_buffers.bvh.UpdateAndExpandBuffers(vkDevice, bvh_, bvh_.size());
                    update_flags_ &= ~kUpdateBvh;
                }

                update_flags_ |= kUpdateNone;
                //compute_.storage_buffers.objects.UpdateAndExpandBuffers(vkDevice, objects, objects.size());
                //compute_.storage_buffers.bvh.UpdateAndExpandBuffers(vkDevice, bvh, bvh.size());
                update_descriptors();
            }
            void Raytracer::update_uniform_buffer(){
                raytracing_component->compute.uniform_buffer.ApplyChanges(vulkan_component->device, raytracing_component->compute.ubo);
            }
            void Raytracer::set_stuff_up()
            {
                camera_.type = Camera::CameraType::lookat;
                camera_.setPerspective(13.0f, 1280.f / 720.f, 0.1f, 1256.0f);
                camera_.setRotation(glm::vec3(35.0f, 90.0f, 45.0f));
                camera_.setTranslation(glm::vec3(0.0f, 0.0f, -4.0f));
                camera_.rotationSpeed = 0.0f;
                camera_.movementSpeed = 7.5f;

                raytracing_component->compute.ubo.aspect_ratio = camera_.aspect;
                //raytracing_component->compute.ubo.lookat = glm::vec3(1.f, 1.f, 1.f);// testScript.vData[6];// camera_.rotation;
                //raytracing_component->compute.ubo.pos = camera_.position * -1.0f;
                raytracing_component->compute.ubo.fov = glm::tan(camera_.fov * 0.03490658503); //0.03490658503 = pi / 180 / 2
                raytracing_component->compute.ubo.rotM = glm::mat4();
                raytracing_component->compute.ubo.rand = random_int();
            }

            #pragma region BoilerPlate
            void Raytracer::create_graphics_pipeline()
            {
                VkPipelineInputAssemblyStateCreateInfo inputAssemblyState =
                    vks::initializers::pipelineInputAssemblyStateCreateInfo(
                        VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
                        0,
                        VK_FALSE);

                VkPipelineRasterizationStateCreateInfo rasterizationState =
                    vks::initializers::pipelineRasterizationStateCreateInfo(
                        VK_POLYGON_MODE_FILL,
                        VK_CULL_MODE_FRONT_BIT,
                        VK_FRONT_FACE_COUNTER_CLOCKWISE,
                        0);

                VkPipelineColorBlendAttachmentState blendAttachmentState =
                    vks::initializers::pipelineColorBlendAttachmentState(
                        0xf,
                        VK_FALSE);

                VkPipelineColorBlendStateCreateInfo colorBlendState =
                    vks::initializers::pipelineColorBlendStateCreateInfo(
                        1,
                        &blendAttachmentState);

                VkPipelineDepthStencilStateCreateInfo depthStencilState =
                    vks::initializers::pipelineDepthStencilStateCreateInfo(
                        VK_FALSE,
                        VK_FALSE,
                        VK_COMPARE_OP_LESS_OR_EQUAL);

                VkPipelineViewportStateCreateInfo viewportState =
                    vks::initializers::pipelineViewportStateCreateInfo(1, 1, 0);

                VkPipelineMultisampleStateCreateInfo multisampleState =
                    vks::initializers::pipelineMultisampleStateCreateInfo(
                        VK_SAMPLE_COUNT_1_BIT,
                        0);

                std::vector<VkDynamicState> dynamicStateEnables = {
                    VK_DYNAMIC_STATE_VIEWPORT,
                    VK_DYNAMIC_STATE_SCISSOR
                };
                VkPipelineDynamicStateCreateInfo dynamicState =
                    vks::initializers::pipelineDynamicStateCreateInfo(
                        dynamicStateEnables.data(),
                        dynamicStateEnables.size(),
                        0);
                auto vertShaderCode = readFile("../Assets/Shaders/texture.vert.spv");
                auto fragShaderCode = readFile("../Assets/Shaders/texture.frag.spv");

                VkShaderModule vertShaderModule;
                VkShaderModule fragShaderModule;

                vertShaderModule = vulkan_component->device.createShaderModule(vertShaderCode);
                fragShaderModule = vulkan_component->device.createShaderModule(fragShaderCode);

                //Create the structure for the vertex shader
                VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
                vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
                vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
                vertShaderStageInfo.module = vertShaderModule;
                vertShaderStageInfo.pName = "main";

                VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
                fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
                fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
                fragShaderStageInfo.module = fragShaderModule;
                fragShaderStageInfo.pName = "main";

                std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages = { vertShaderStageInfo, fragShaderStageInfo };

                VkGraphicsPipelineCreateInfo pipelineCreateInfo =
                    vks::initializers::pipelineCreateInfo(
                        raytracing_component->graphics.pipeline_layout,
                        renderPass,
                        0);

                VkPipelineVertexInputStateCreateInfo emptyInputState{};
                emptyInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
                emptyInputState.vertexAttributeDescriptionCount = 0;
                emptyInputState.pVertexAttributeDescriptions = nullptr;
                emptyInputState.vertexBindingDescriptionCount = 0;
                emptyInputState.pVertexBindingDescriptions = nullptr;
                pipelineCreateInfo.pVertexInputState = &emptyInputState;

                pipelineCreateInfo.pInputAssemblyState = &inputAssemblyState;
                pipelineCreateInfo.pRasterizationState = &rasterizationState;
                pipelineCreateInfo.pColorBlendState = &colorBlendState;
                pipelineCreateInfo.pMultisampleState = &multisampleState;
                pipelineCreateInfo.pViewportState = &viewportState;
                pipelineCreateInfo.pDepthStencilState = &depthStencilState;
                pipelineCreateInfo.pDynamicState = &dynamicState;
                pipelineCreateInfo.stageCount = shaderStages.size();
                pipelineCreateInfo.pStages = shaderStages.data();
                pipelineCreateInfo.renderPass = renderPass;

                Log::check(VK_SUCCESS == vkCreateGraphicsPipelines(vulkan_component->device.logical, pipelineCache, 1, &pipelineCreateInfo, nullptr, &raytracing_component->graphics.pipeline), "CREATE GRAPHICS PIPELINE");

                //must be destroyed at the end of the object
                vkDestroyShaderModule(vulkan_component->device.logical, fragShaderModule, nullptr);
                vkDestroyShaderModule(vulkan_component->device.logical, vertShaderModule, nullptr);
            }
            void Raytracer::create_descriptor_pool()
            {
                std::vector<VkDescriptorPoolSize> poolSizes =
                {
                    vks::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2),			// Compute UBO
                    vks::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 3 + MAX_TEXTURES),	// Graphics image samplers || +4 FOR TEXTURE
                    vks::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1),				// Storage image for ray traced image output
                    vks::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 9),			// Storage buffer for the scene primitives
                    //vks::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1)
                };

                VkDescriptorPoolCreateInfo descriptorPoolInfo =
                    vks::initializers::descriptorPoolCreateInfo(
                        poolSizes.size(),
                        poolSizes.data(),
                        3);

                Log::check(VK_SUCCESS == vkCreateDescriptorPool(vulkan_component->device.logical, &descriptorPoolInfo, nullptr, &descriptor_pool_), "CREATE DESCRIPTOR POOL");
            }
            void Raytracer::create_descriptor_sets()
            {
                VkDescriptorSetAllocateInfo allocInfo =
                vks::initializers::descriptorSetAllocateInfo(
                    descriptor_pool_,
                    &raytracing_component->graphics.descriptor_set_layout,
                    1);

                Log::check(VK_SUCCESS == vkAllocateDescriptorSets(vulkan_component->device.logical, &allocInfo, &raytracing_component->graphics.descriptor_set), "ALLOCATE DESCRIPTOR SET");

                std::vector<VkWriteDescriptorSet> writeDescriptorSets =
                {
                    // Binding 0 : Fragment shader texture sampler
                    vks::initializers::writeDescriptorSet(
                        raytracing_component->graphics.descriptor_set,
                        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                        0,
                        &compute_texture_.descriptor)
                };

                vkUpdateDescriptorSets(vulkan_component->device.logical, writeDescriptorSets.size(), writeDescriptorSets.data(), 0, NULL);
            }
            void Raytracer::create_descriptor_set_layout()
            {
                std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings =
                {
                    // Binding 0 : Fragment shader image sampler
                    vks::initializers::descriptorSetLayoutBinding(
                        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                        VK_SHADER_STAGE_FRAGMENT_BIT,
                        0)
                };

                VkDescriptorSetLayoutCreateInfo descriptorLayout =
                    vks::initializers::descriptorSetLayoutCreateInfo(
                        setLayoutBindings.data(),
                        setLayoutBindings.size());

                Log::check(VK_SUCCESS == vkCreateDescriptorSetLayout(vulkan_component->device.logical, &descriptorLayout, nullptr, &raytracing_component->graphics.descriptor_set_layout), "CREATE DESCRIPTOR SET LAYOUT");

                VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo =
                    vks::initializers::pipelineLayoutCreateInfo(
                        &raytracing_component->graphics.descriptor_set_layout,
                        1);

                Log::check(VK_SUCCESS == vkCreatePipelineLayout(vulkan_component->device.logical, &pPipelineLayoutCreateInfo, nullptr, &raytracing_component->graphics.pipeline_layout), "CREATE PIPELINE LAYOUT");
            }
            void Raytracer::create_command_buffers(float swap_ratio, int32_t offset_width, int32_t offset_heigiht)
            {
                commandBuffers.resize(vulkan_component->swapchain.frame_buffers.size());
                UpdateSwapScale(swap_ratio);
                VkCommandBufferAllocateInfo allocInfo = {};
                allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
                allocInfo.commandPool = commandPool;
                allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY; //specifies if its a primary or secondary buffer
                allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();

                //@COMPUTEHERE be sure to have compute-specific command buffers too
                if (vkAllocateCommandBuffers(vulkan_component->device.logical, &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
                    throw std::runtime_error("failed to allocate command buffers!");
                }


                for (size_t i = 0; i < commandBuffers.size(); i++) {
                    VkCommandBufferBeginInfo beginInfo = {};
                    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
                    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT; //The cmdbuf will be rerecorded right after executing it 1s
                    beginInfo.pInheritanceInfo = nullptr; // Optional //only for secondary buffers

                    vkBeginCommandBuffer(commandBuffers[i], &beginInfo);
                    // Image memory barrier to make sure that compute shader writes are finished before sampling from the texture
                    VkImageMemoryBarrier imageMemoryBarrier = {};
                    imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
                    imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
                    imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
                    imageMemoryBarrier.image = compute_texture_.image;
                    imageMemoryBarrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
                    imageMemoryBarrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
                    imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
                    vkCmdPipelineBarrier(
                        commandBuffers[i],
                        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                        0,
                        0, nullptr,
                        0, nullptr,
                        1, &imageMemoryBarrier);

                    VkRenderPassBeginInfo renderPassInfo = {};
                    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
                    renderPassInfo.renderPass = renderPass;
                    renderPassInfo.framebuffer = vulkan_component->swapchain.frame_buffers[i];
                    renderPassInfo.renderArea.offset = { offset_width, offset_height }; //size of render area, should match size of attachments
                    renderPassInfo.renderArea.extent = scaled_swap_;// swapChainExtent; //scaledSwap;//

                    std::array<VkClearValue, 2> clearValues = {};
                    clearValues[0].color = { 0.0f, 0.0f, 0.0f, 1.0f }; //derp
                    clearValues[1].depthStencil = { 1.0f, 0 }; //1.0 = farplane, 0.0 = nearplane HELP

                    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size()); //cuz
                    renderPassInfo.pClearValues = clearValues.data(); //duh

                    vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

                    VkViewport viewport = vks::initializers::viewport(swapChainExtent.width, swapChainExtent.height, 0.0f, 1.0f);
                    vkCmdSetViewport(commandBuffers[i], 0, 1, &viewport);

                    VkRect2D scissor = vks::initializers::rect2D(swapChainExtent.width, swapChainExtent.height, 0, 0);
                    vkCmdSetScissor(commandBuffers[i], 0, 1, &scissor);

                    vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, raytracing_component->graphics.pipeline_layout, 0, 1, &raytracing_component->graphics.descriptor_set, 0, NULL);
                    vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, raytracing_component->graphics.pipeline);
                    vkCmdDraw(commandBuffers[i], 3, 1, 0, 0);
                    vkCmdEndRenderPass(commandBuffers[i]);

                    Log::check(VK_SUCCESS == vkEndCommandBuffer(commandBuffers[i]), "END COMMAND BUFFER");
                }
            }
            void Raytracer::create_compute_command_buffers()
            {
                VkCommandBufferBeginInfo cmdBufInfo = vks::initializers::commandBufferBeginInfo();
                Log::check(VK_SUCCESS == vkBeginCommandBuffer(raytracing_component->compute.command_buffer, &cmdBufInfo), "CREATE COMPUTE COMMAND BUFFER");
                vkCmdBindPipeline(raytracing_component->compute.command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, raytracing_component->compute.pipeline);
                vkCmdBindDescriptorSets(raytracing_component->compute.command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, raytracing_component->compute.pipeline_layout, 0, 1, &raytracing_component->compute.descriptor_set, 0, 0);
                vkCmdDispatch(raytracing_component->compute.command_buffer, compute_texture_.width / 16, compute_texture_.height / 16, 1);
                vkEndCommandBuffer(raytracing_component->compute.command_buffer);
            }
            void Raytracer::create_uniform_buffers()
            {
                raytracing_component->compute.uniform_buffer.Initialize(vulkan_component->device, 1,
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
                raytracing_component->compute.uniform_buffer.ApplyChanges(vulkan_component->device, raytracing_component->compute.ubo);
            }
            void Raytracer::prepare_storage_buffers()
            {
                compute_component->shader_data.materials.reserve(MAX_MATERIALS);
                lights_.reserve(MAX_LIGHTS);

                //these are changable 
                compute_component->storage_buffers.primitives.InitStorageBufferCustomSize(vulkan_component->device, primitives_, primitives_.size(), MAX_OBJS);
                compute_component->storage_buffers.materials.InitStorageBufferCustomSize(vulkan_component->device, materials_, compute_component->shader_data.materials.size(), MAX_MATERIALS);
                compute_component->storage_buffers.lights.InitStorageBufferCustomSize(vulkan_component->device, lights_, lights_.size(), MAX_LIGHTS);

                //create 1 gui main global kind of gui for like title/menu screen etc...
                GUIComponent* guiComp = (GUIComponent*)world->getSingleton()->getComponent<GUIComponent>();
                ssGUI gui = ssGUI(guiComp->min, guiComp->extents, guiComp->alignMin, guiComp->alignExt, guiComp->layer, guiComp->id);
                gui.alpha = guiComp->alpha;

                //Give the component a reference to it and initialize
                guiComp->ref = guis_.size();
                guis_.push_back(gui);
                compute_component->storage_buffers.guis.InitStorageBufferCustomSize(vulkan_component->device, guis_, guis_.size(), MAX_GUIS);
                compute_component->storage_buffers.bvh.InitStorageBufferCustomSize(vulkan_component->device, bvh_, bvh_.size(), MAX_NODES);
            }
            void Raytracer::prepare_texture_target(Texture *tex, uint32_t width, uint32_t height, VkFormat format)
            {
                // Get device properties for the requested texture format
                VkFormatProperties formatProperties;
                
                vkGetPhysicalDeviceFormatProperties(vulkan_component->device.physical, format, &formatProperties);
                // Check if requested image format supports image storage operations
                assert(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT);

                // Prepare blit target texture
                tex->width = width;
                tex->height = height;

                VkImageCreateInfo imageCreateInfo = vks::initializers::imageCreateInfo();
                imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
                imageCreateInfo.format = format;
                imageCreateInfo.extent = { width, height, 1 };
                imageCreateInfo.mipLevels = 1;
                imageCreateInfo.arrayLayers = 1;
                imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
                imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
                imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                // Image will be sampled in the fragment shader and used as storage target in the compute shader
                imageCreateInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
                imageCreateInfo.flags = 0;

                VkMemoryAllocateInfo memAllocInfo = vks::initializers::memoryAllocateInfo();
                VkMemoryRequirements memReqs;

                Log::check(VK_SUCCESS == vkCreateImage(vulkan_component->device.logical, &imageCreateInfo, nullptr, &tex->image), "CREATE IMAGE");
                vkGetImageMemoryRequirements(vulkan_component->device.logical, tex->image, &memReqs);
                memAllocInfo.allocationSize = memReqs.size;
                memAllocInfo.memoryTypeIndex = vulkan_component->device.findMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
                Log::check(VK_SUCCESS == vkAllocateMemory(vulkan_component->device.logical, &memAllocInfo, nullptr, &tex->memory), "ALLOCATE TXTR MEMORY");
                Log::check(VK_SUCCESS == vkBindImageMemory(vulkan_component->device.logical, tex->image, tex->memory, 0), "BIND IMAGE MEMORY");

                VkCommandBuffer layoutCmd = vulkan_component->device.beginSingleTimeCommands(); //VulkanExampleBase::createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

                tex->imageLayout = VK_IMAGE_LAYOUT_GENERAL;
                vulkan_component->device.setImageLayout(
                    layoutCmd,
                    tex->image,
                    VK_IMAGE_ASPECT_COLOR_BIT,
                    VK_IMAGE_LAYOUT_UNDEFINED,
                    tex->imageLayout);

                vulkan_component->device.endSingleTimeCommands(layoutCmd); //VulkanExampleBase::flushCommandBuffer(layoutCmd, queue, true);

                // Create sampler
                VkSamplerCreateInfo sampler = vks::initializers::samplerCreateInfo();
                sampler.magFilter = VK_FILTER_LINEAR;
                sampler.minFilter = VK_FILTER_LINEAR;
                sampler.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
                sampler.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
                sampler.addressModeV = sampler.addressModeU;
                sampler.addressModeW = sampler.addressModeU;
                sampler.mipLodBias = 0.0f;
                sampler.maxAnisotropy = 1.0f;
                sampler.compareOp = VK_COMPARE_OP_NEVER;
                sampler.minLod = 0.0f;
                sampler.maxLod = 0.0f;
                sampler.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
                Log::check(VK_SUCCESS == vkCreateSampler(vulkan_component->device.logical, &sampler, nullptr, &tex->sampler), "CREATE SAMPLER");

                // Create image view
                VkImageViewCreateInfo view = vks::initializers::imageViewCreateInfo();
                view.viewType = VK_IMAGE_VIEW_TYPE_2D;
                view.format = format;
                view.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
                view.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
                view.image = tex->image;
                Log::check(VK_SUCCESS == vkCreateImageView(vulkan_component->device.logical, &view, nullptr, &tex->view), "CREATE IMAGE");

                // Initialize a descriptor for later use
                tex->descriptor.imageLayout = tex->imageLayout;
                tex->descriptor.imageView = tex->view;
                tex->descriptor.sampler = tex->sampler;
                //tex->device = vulkanDevice;
            }
            void Raytracer::prepare_compute()
            {
                // Create a compute capable device queue
                // The VulkanDevice::createLogicalDevice functions finds a compute capable queue and prefers queue families that only support compute
                // Depending on the implementation this may result in different queue family indices for graphics and computes,
                // requiring proper synchronization (see the memory barriers in buildComputeCommandBuffer)
                VkDeviceQueueCreateInfo queueCreateInfo = {};
                queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
                queueCreateInfo.pNext = NULL;
                queueCreateInfo.queueFamilyIndex = vulkan_component->device.qFams.computeFamily;
                queueCreateInfo.queueCount = 1;
                vkGetDeviceQueue(vulkan_component->device.logical, vulkan_component->device.qFams.computeFamily, 0, &raytracing_component->compute.queue);

                std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = {
                    // Binding 0: Storage image (raytraced output)
                    vks::initializers::descriptorSetLayoutBinding(
                        VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
                        VK_SHADER_STAGE_COMPUTE_BIT,
                        0),
                    // Binding 1: Uniform buffer block
                    vks::initializers::descriptorSetLayoutBinding(
                        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                        VK_SHADER_STAGE_COMPUTE_BIT,
                        1),
                    // Binding 2: Shader storage buffer for the verts
                    vks::initializers::descriptorSetLayoutBinding(
                        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                        VK_SHADER_STAGE_COMPUTE_BIT,
                        2),
                    // Binding 3: Shader storage buffer for the indices
                    vks::initializers::descriptorSetLayoutBinding(
                        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                        VK_SHADER_STAGE_COMPUTE_BIT,
                        3),
                    // Binding 4: Shader storage buffer for the blas
                    vks::initializers::descriptorSetLayoutBinding(
                        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                        VK_SHADER_STAGE_COMPUTE_BIT,
                        4),
                    // Binding 5: Shader storage buffer for the shapes
                    vks::initializers::descriptorSetLayoutBinding(
                        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                        VK_SHADER_STAGE_COMPUTE_BIT,
                        5),
                    // Binding 6: Shader storage buffer for the objects
                    vks::initializers::descriptorSetLayoutBinding(
                        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                        VK_SHADER_STAGE_COMPUTE_BIT,
                        6),
                    // Binding 7: Shader storage buffer for the materials
                    vks::initializers::descriptorSetLayoutBinding(
                        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                        VK_SHADER_STAGE_COMPUTE_BIT,
                        7),
                    // Binding 8: Shader storage buffer for the lights
                    vks::initializers::descriptorSetLayoutBinding(
                        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                        VK_SHADER_STAGE_COMPUTE_BIT,
                        8),
                    // binding 9: Shader storage buffer for the guis_
                    vks::initializers::descriptorSetLayoutBinding(
                        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                        VK_SHADER_STAGE_COMPUTE_BIT,
                        9),
                    // Binding 10: Shader storage buffer for the bvh
                    vks::initializers::descriptorSetLayoutBinding(
                        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                        VK_SHADER_STAGE_COMPUTE_BIT,
                        10),
                    // Binding 12: the textures
                    vks::initializers::descriptorSetLayoutBinding(
                        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                        VK_SHADER_STAGE_COMPUTE_BIT,
                        11, MAX_TEXTURES)
                };

                VkDescriptorSetLayoutCreateInfo descriptorLayout =
                    vks::initializers::descriptorSetLayoutCreateInfo(
                        setLayoutBindings.data(),
                        setLayoutBindings.size());

                Log::check(VK_SUCCESS == vkCreateDescriptorSetLayout(vulkan_component->device.logical, &descriptorLayout, nullptr, &raytracing_component->compute.descriptor_set_layout), "CREATE COMPUTE DSL");

                VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo =
                    vks::initializers::pipelineLayoutCreateInfo(
                        &raytracing_component->compute.descriptor_set_layout,
                        1);

                Log::check(VK_SUCCESS == vkCreatePipelineLayout(vulkan_component->device.logical, &pPipelineLayoutCreateInfo, nullptr, &raytracing_component->compute.pipeline_layout), "CREATECOMPUTE PIEPLINEEEE");

                VkDescriptorSetAllocateInfo allocInfo =
                    vks::initializers::descriptorSetAllocateInfo(
                        descriptor_pool_,
                        &raytracing_component->compute.descriptor_set_layout,
                        1);

                Log::check(VK_SUCCESS == vkAllocateDescriptorSets(vulkan_component->device.logical, &allocInfo, &raytracing_component->compute.descriptor_set), "ALLOCATE DOMPUTE DSET");

                VkDescriptorImageInfo textureimageinfos[MAX_TEXTURES] = {
                    gui_textures_[0].descriptor,
                    gui_textures_[1].descriptor,
                    gui_textures_[2].descriptor,
                    gui_textures_[3].descriptor,
                    gui_textures_[4].descriptor
                };
                compute_write_descriptor_sets_ =
                {
                    // Binding 0: Output storage image
                    vks::initializers::writeDescriptorSet(
                        raytracing_component->compute.descriptor_set,
                        VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
                        0,
                        &compute_texture_.descriptor),
                    // Binding 1: Uniform buffer block
                    vks::initializers::writeDescriptorSet(
                        raytracing_component->compute.descriptor_set,
                        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                        1,
                        &raytracing_component->compute.uniform_buffer.bufferInfo),
                    // Binding 2: Shader storage buffer for the verts
                    vks::initializers::writeDescriptorSet(
                        raytracing_component->compute.descriptor_set,
                        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                        2,
                        &compute_component->storage_buffers.verts.bufferInfo),
                    // Binding 3: Shader storage buffer for the indices
                    vks::initializers::writeDescriptorSet(
                        raytracing_component->compute.descriptor_set,
                        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                        3,
                        &compute_component->storage_buffers.faces.bufferInfo),
                    // Binding 4: for blas
                    vks::initializers::writeDescriptorSet(
                        raytracing_component->compute.descriptor_set,
                        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                        4,
                        &compute_component->storage_buffers.blas.bufferInfo),
                    //Binding 5: for shapes
                    vks::initializers::writeDescriptorSet(
                        raytracing_component->compute.descriptor_set,
                        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                        5,
                        &compute_component->storage_buffers.shapes.bufferInfo),
                    // Binding 6: for objectss
                    vks::initializers::writeDescriptorSet(
                        raytracing_component->compute.descriptor_set,
                        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                        6,
                        &compute_component->storage_buffers.primitives.bufferInfo),
                    //Binding 8 for materials
                    vks::initializers::writeDescriptorSet(
                        raytracing_component->compute.descriptor_set,
                        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                        7,
                        &compute_component->storage_buffers.materials.bufferInfo),
                    //Binding 9 for lights
                    vks::initializers::writeDescriptorSet(
                        raytracing_component->compute.descriptor_set,
                        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                        8,
                        &compute_component->storage_buffers.lights.bufferInfo),
                    //Binding 10 for guis_
                    vks::initializers::writeDescriptorSet(
                        raytracing_component->compute.descriptor_set,
                        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                        9,
                        &compute_component->storage_buffers.guis.bufferInfo),
                    //Binding 11 for bvhs
                    vks::initializers::writeDescriptorSet(
                        raytracing_component->compute.descriptor_set,
                        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                        10,
                        &compute_component->storage_buffers.bvh.bufferInfo),
                    //bINDING 12 FOR TEXTURES
                    vks::initializers::writeDescriptorSet(
                        raytracing_component->compute.descriptor_set,
                        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                        11,
                        textureimageinfos, MAX_TEXTURES)
                };

                vkUpdateDescriptorSets(vulkan_component->device.logical, compute_write_descriptor_sets_.size(), compute_write_descriptor_sets_.data(), 0, NULL);

                // Create compute shader pipelines
                VkComputePipelineCreateInfo computePipelineCreateInfo =
                    vks::initializers::computePipelineCreateInfo(
                        raytracing_component->compute.pipeline_layout,
                        0);

                computePipelineCreateInfo.stage = vulkan_component->device.createShader("../Assets/Shaders/raytracing.comp.spv", VK_SHADER_STAGE_COMPUTE_BIT);
                Log::check(VK_SUCCESS == vkCreateComputePipelines(vulkan_component->device.logical, pipelineCache, 1, &computePipelineCreateInfo, nullptr, &raytracing_component->compute.pipeline), "CREATE COMPUTE PIPELINE");

                // Separate command pool as queue family for compute may be different than graphics
                VkCommandPoolCreateInfo cmdPoolInfo = {};
                cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
                cmdPoolInfo.queueFamilyIndex = vulkan_component->device.qFams.computeFamily;
                cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
                Log::check(VK_SUCCESS == vkCreateCommandPool(vulkan_component->device.logical, &cmdPoolInfo, nullptr, &raytracing_component->compute.command_pool), "CREATE COMMAND POOL");

                // Create a command buffer for compute operations
                VkCommandBufferAllocateInfo cmdBufAllocateInfo =
                    vks::initializers::commandBufferAllocateInfo(
                        raytracing_component->compute.command_pool,
                        VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                        1);

                Log::check(VK_SUCCESS == vkAllocateCommandBuffers(vulkan_component->device.logical, &cmdBufAllocateInfo, &raytracing_component->compute.command_buffer), "ALLOCATE COMMAND BUFFERS");

                // Fence for compute CB sync
                VkFenceCreateInfo fenceCreateInfo = vks::initializers::fenceCreateInfo(VK_FENCE_CREATE_SIGNALED_BIT);
                Log::check(VK_SUCCESS == vkCreateFence(vulkan_component->device.logical, &fenceCreateInfo, nullptr, &raytracing_component->compute.fence), "CREATE FENCE");

                // Build a single command buffer containing the compute dispatch commands
                CreateComputeCommandBuffer();
                vkDestroyShaderModule(vulkan_component->device.logical, computePipelineCreateInfo.stage.module, nullptr);
            }
            void Raytracer::destroy_compute()
            {
                raytracing_component->compute.uniform_buffer.Destroy(vulkan_component->device);
                compute_component->storage_buffers.verts.Destroy(vulkan_component->device);
                compute_component->storage_buffers.faces.Destroy(vulkan_component->device);
                compute_component->storage_buffers.blas.Destroy(vulkan_component->device);
                compute_component->storage_buffers.shapes.Destroy(vulkan_component->device);
                compute_component->storage_buffers.primitives.Destroy(vulkan_component->device);
                compute_component->storage_buffers.materials.Destroy(vulkan_component->device);
                compute_component->storage_buffers.lights.Destroy(vulkan_component->device);
                compute_component->storage_buffers.guis.Destroy(vulkan_component->device);
                compute_component->storage_buffers.bvh.Destroy(vulkan_component->device);

                raytracing_component->compute_texture.destroy(vulkan_component->device.logical);
                for (int i = 0; i < MAX_TEXTURES; ++i)
                    gui_textures_[i].destroy(vulkan_component->device.logical);

                vkDestroyPipelineCache(vulkan_component->device.logical, pipelineCache, nullptr);
                vkDestroyPipeline(vulkan_component->device.logical, raytracing_component->compute.pipeline, nullptr);
                vkDestroyPipelineLayout(vulkan_component->device.logical, raytracing_component->compute.pipeline_layout, nullptr);
                vkDestroyDescriptorSetLayout(vulkan_component->device.logical, raytracing_component->compute.descriptor_set_layout, nullptr);
                vkDestroyFence(vulkan_component->device.logical, raytracing_component->compute.fence, nullptr);
                vkDestroyCommandPool(vulkan_component->device.logical, raytracing_component->compute.command_pool, nullptr);
            }
            #pragma endregion
        }
    }
}