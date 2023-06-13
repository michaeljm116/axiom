#include "pch.h"
#include "render-base.h"
#include "sys-log.h"
namespace axiom
{
    namespace render
    {
        namespace base
        {

            void InitializeVulkan(Cmp_Vulkan& vulkan_component){
                RenderBase base;
                base.vulkan_component = &vulkan_component;
                base.initVulkan();
            };

            void RenderBase::initWindow() {
                //Handle swapchain recreation
                auto* window = g_world.get_mut<Cmp_Window>();
                glfwSetWindowUserPointer(window->window, this);
                glfwSetWindowSizeCallback(window->window, RenderBase::onWindowResized);
            }
            void RenderBase::initVulkan() {
                //initWindow();
                createInstance();
                vulkan_component->device.setupDebugCallback();
                createSurface();
                pickPhysicalDevice();
                createLogicalDevice();
                createSwapChain();
                createImageViews();
                createRenderPass();
                createPipelineCache();
                createCommandPool();
                createDepthResources();
                createFrameBuffers();
                createSemaphores();
            }

            void RenderBase::cleanup() {
                vkDestroySemaphore(vulkan_component->device.logical, vulkan_component->semaphores.render_finished, nullptr);
                vkDestroySemaphore(vulkan_component->device.logical, vulkan_component->semaphores.image_available, nullptr);
                vkDestroySurfaceKHR(vulkan_component->device.instance, vulkan_component->swapchain.surface, nullptr);

                vulkan_component->device.Destroy();
            }
            void RenderBase::cleanupSwapChain() {
                vkDestroyImageView(vulkan_component->device.logical, vulkan_component->depth.image_view, nullptr);
                vkDestroyImage(vulkan_component->device.logical, vulkan_component->depth.image, nullptr);
                vkFreeMemory(vulkan_component->device.logical, vulkan_component->depth.image_memory, nullptr);

                for (size_t i = 0; i < vulkan_component->swapchain.frame_buffers.size(); i++) {
                    vkDestroyFramebuffer(vulkan_component->device.logical, vulkan_component->swapchain.frame_buffers[i], nullptr);
                }

                //vkFreeCommandBuffers(vulkan_component->device.logical, commandPool, static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());

                vkDestroyRenderPass(vulkan_component->device.logical, vulkan_component->pipeline.render_pass, nullptr);

                for (size_t i = 0; i < vulkan_component->swapchain.image_views.size(); i++) {
                    vkDestroyImageView(vulkan_component->device.logical, vulkan_component->swapchain.image_views[i], nullptr);
                }

                vkDestroySwapchainKHR(vulkan_component->device.logical, vulkan_component->swapchain.get, nullptr);
            }

            void RenderBase::createInstance() {

                //Check validation layers
                vulkan_component->device.validation_enabled = vulkan_component->device.enableValidationLayers && vulkan_component->device.checkValidationLayerSupport();
                log::Check(vulkan_component->device.validation_enabled, "Enable Validation Layer");

                //Optional Data about the application
                VkApplicationInfo appInfo = {};
                appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
                appInfo.pApplicationName = "Hello Principium Cognoscendi";
                appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
                appInfo.pEngineName = "Axiom Engine";
                appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
                appInfo.apiVersion = VK_API_VERSION_1_0;

                //Required struct to tell driver which extentions/validation layers to use
                //*Pointer to Struct with creation Info
                VkInstanceCreateInfo createInfo = {};
                createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
                createInfo.pApplicationInfo = &appInfo;

                //Check for extensions
                auto extensions = getRequiredExtentions();
                createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
                createInfo.ppEnabledExtensionNames = extensions.data();

                //Check for validation layers, else 0
                if (vulkan_component->device.validation_enabled) {
                    createInfo.enabledLayerCount = static_cast<uint32_t>(vulkan_component->device.validationLayers.size());
                    createInfo.ppEnabledLayerNames = vulkan_component->device.validationLayers.data();
                }
                else
                    createInfo.enabledLayerCount = 0;

                //*Pointer to custom allocator callbacks
                //*Pointer to the variable that stores the handle to the new object
                VkResult result = vkCreateInstance(&createInfo, nullptr, &vulkan_component->device.instance);

                if (result != VK_SUCCESS)
                    throw std::runtime_error("failed to create instance!");

                    volkLoadInstance(vulkan_component->device.instance);
            }
            void RenderBase::createSurface() {
                auto* g_window = g_world.get_mut<Cmp_Window>();
                if (glfwCreateWindowSurface(vulkan_component->device.instance, g_window->window, nullptr, &vulkan_component->swapchain.surface) != VK_SUCCESS) {
                    throw std::runtime_error("failed to create window surface!");
                }
            }
            void RenderBase::createLogicalDevice() {

                vulkan_component->device.qFams = findQueueFamilies(vulkan_component->device.physical);
                setComputeQueueFamilyIndex();
                //ALSO make sure it can present to the screen y0
                std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
                std::set<int> uniqueQueueFamilies = { vulkan_component->device.qFams.graphicsFamily, vulkan_component->device.qFams.presentFamily, vulkan_component->device.qFams.computeFamily };

                // 0.0 -> 1.0 priority for scheduling of command buffer execution
                float queuePriority[] = { 1.0f, 0.0f };

                for (int queueFamily : uniqueQueueFamilies) {
                    //Make sure we get a queue with assigned capabilities
                    VkDeviceQueueCreateInfo queueCreateInfo = {};
                    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
                    queueCreateInfo.queueFamilyIndex = queueFamily;
                    //if (queueFamily == qFams.computeFamily)
                    //	queueCreateInfo.queueCount = 1;
                    //else
                    queueCreateInfo.queueCount = 2;
                    queueCreateInfo.pQueuePriorities = queuePriority;

                    //push it into a the set yo
                    queueCreateInfos.push_back(queueCreateInfo);
                }

                //The set of device features we'll be using
                VkPhysicalDeviceFeatures deviceFeatures = {};
                deviceFeatures.samplerAnisotropy = VK_TRUE;

                //Creating the logical device now yo
                VkDeviceCreateInfo createInfo = {};
                createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
                createInfo.pQueueCreateInfos = queueCreateInfos.data();
                createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
                createInfo.pEnabledFeatures = &deviceFeatures;

                //enable extensions
                createInfo.enabledExtensionCount = static_cast<uint32_t>(vulkan_component->device.deviceExtensions.size());
                createInfo.ppEnabledExtensionNames = vulkan_component->device.deviceExtensions.data();

                //Check validation layers
                if (vulkan_component->device.validation_enabled) {
                    createInfo.enabledLayerCount = static_cast<uint32_t>(vulkan_component->device.validationLayers.size());
                    createInfo.ppEnabledLayerNames = vulkan_component->device.validationLayers.data();
                }
                else {
                    createInfo.enabledLayerCount = 0;
                }

                //CREATE DAT DERR DEVICE OVA YONDER YA HEARD?
                if (vkCreateDevice(vulkan_component->device.physical, &createInfo, nullptr, &vulkan_component->device.logical) != VK_SUCCESS) {
                    throw std::runtime_error("failed to create logical device!");
                }
                volkLoadDevice(vulkan_component->device.logical);

                vkGetDeviceQueue(vulkan_component->device.logical, vulkan_component->device.qFams.graphicsFamily, 0, &vulkan_component->queues.graphics);
                vkGetDeviceQueue(vulkan_component->device.logical, vulkan_component->device.qFams.presentFamily, 0, &vulkan_component->queues.present);
                vkGetDeviceQueue(vulkan_component->device.logical, vulkan_component->device.qFams.computeFamily, 1, &vulkan_component->queues.compute); //might be a 0 or a 2? idk???

                //set vkdevice queue tyighnny
                vulkan_component->device.queue = &vulkan_component->queues.graphics;
            }
            void RenderBase::createSwapChain() {
                SwapChainSupportDetails swapChainSupport = querySwapChainSupport(vulkan_component->device.physical);

                VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
                VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
                VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

                //find out the number of images in the vulkan_component->swapchain.get, 0 = no limit
                uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
                if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
                    imageCount = swapChainSupport.capabilities.maxImageCount;
                }

                //set up the surface of the sc
                VkSwapchainCreateInfoKHR createInfo = {};
                createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
                createInfo.surface = vulkan_component->swapchain.surface;

                //details of the swap chain
                createInfo.minImageCount = imageCount;
                createInfo.imageFormat = surfaceFormat.format;
                createInfo.imageColorSpace = surfaceFormat.colorSpace;
                createInfo.imageExtent = extent;
                createInfo.imageArrayLayers = 1; //layers of image, always 1 unless 3D
                createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; //VK_IMAGE_USAGE_TRANSFER_DST_BIT for post-processing

                                                                            //Specify how to handle swap chain images across multiple queue fams
                vulkan::QueueFamilyIndices indices = findQueueFamilies(vulkan_component->device.physical);
                uint32_t queueFamilyIndices[] = { (uint32_t)indices.graphicsFamily, (uint32_t)indices.presentFamily };

                if (indices.graphicsFamily != indices.presentFamily) {
                    createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT; //multiple queues can be @COMPUTEHERE, need to share 
                    createInfo.queueFamilyIndexCount = 2;
                    createInfo.pQueueFamilyIndices = queueFamilyIndices;
                }
                else {
                    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE; //1 queue at a time but best perf
                                                                            //createInfo.queueFamilyIndexCount = 0; // Optional
                                                                            //createInfo.pQueueFamilyIndices = nullptr; // Optional
                }

                //specify  the transform, like a 90deg rotation or a flip
                createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
                //specify if the alpha should be used for blending iwth other windows
                createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
                //enable clipping
                createInfo.presentMode = presentMode;
                createInfo.clipped = VK_TRUE;

                //old swapchain incase the current one changes
                //createInfo.oldSwapchain = VK_NULL_HANDLE;


                if (vkCreateSwapchainKHR(vulkan_component->device.logical, &createInfo, nullptr, &vulkan_component->swapchain.get) != VK_SUCCESS) {
                    throw std::runtime_error("failed to create swapchain!");
                }

                vkGetSwapchainImagesKHR(vulkan_component->device.logical, vulkan_component->swapchain.get, &imageCount, nullptr);
                vulkan_component->swapchain.images.resize(imageCount);
                vkGetSwapchainImagesKHR(vulkan_component->device.logical, vulkan_component->swapchain.get, &imageCount, vulkan_component->swapchain.images.data());

                vulkan_component->swapchain.image_format = surfaceFormat.format;
                vulkan_component->swapchain.extent = extent;
            }
            void RenderBase::createImageViews() {
                vulkan_component->swapchain.image_views.resize(vulkan_component->swapchain.images.size());

                for (uint32_t i = 0; i < vulkan_component->swapchain.images.size(); i++) {
                    vulkan_component->swapchain.image_views[i] = vulkan_component->device.createImageView(vulkan_component->swapchain.images[i], vulkan_component->swapchain.image_format, VK_IMAGE_ASPECT_COLOR_BIT);
                }
            }
            void RenderBase::createRenderPass() {
                VkAttachmentDescription colorAttachment = {};
                colorAttachment.format = vulkan_component->swapchain.image_format;
                colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT; //no sammples
                colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; //Clears frambuffer before drawing new frame
                colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE; //stores the image in memory to be read later
                colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; //don't care about what the previous layout of image was
                colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; //image to be presented to swapchain;

                VkAttachmentReference colorAttachmentRef = {};
                colorAttachmentRef.attachment = 0;  //we only have 1 description so it'll be array index 0
                colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; //best performance?!?

                VkAttachmentDescription depthAttachment = {};
                depthAttachment.format = findDepthFormat();
                depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
                depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
                depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

                VkAttachmentReference depthAttachmentRef = {};
                depthAttachmentRef.attachment = 1;
                depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

                VkSubpassDescription subpass = {};
                subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS; //let it know its a graphics subpass @COMPUTEHERE
                subpass.colorAttachmentCount = 1;
                subpass.pColorAttachments = &colorAttachmentRef;
                subpass.pDepthStencilAttachment = &depthAttachmentRef;

                VkSubpassDependency dependency = {};
                dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
                dependency.dstSubpass = 0;
                dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
                dependency.srcAccessMask = 0;
                dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
                dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

                std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };
                VkRenderPassCreateInfo renderPassInfo = {};
                renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
                renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
                renderPassInfo.pAttachments = attachments.data();
                renderPassInfo.subpassCount = 1;
                renderPassInfo.pSubpasses = &subpass;
                renderPassInfo.dependencyCount = 1;
                renderPassInfo.pDependencies = &dependency;

                if (vkCreateRenderPass(vulkan_component->device.logical, &renderPassInfo, nullptr, &vulkan_component->pipeline.render_pass) != VK_SUCCESS) {
                    throw std::runtime_error("failed to create render pass!");
                }
            }
            void RenderBase::createPipelineCache()
            {
                VkPipelineCacheCreateInfo createInfo = {};
                createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
                if (vkCreatePipelineCache(vulkan_component->device.logical, &createInfo, nullptr, &vulkan_component->pipeline.cache) != VK_SUCCESS)
                    std::runtime_error("failed to create pipelinecache!");
            }
            void RenderBase::createDepthResources() {
                VkFormat depthFormat = findDepthFormat();
                vulkan_component->device.createImage(vulkan_component->swapchain.extent.width, vulkan_component->swapchain.extent.height, vulkan_component->depth.format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vulkan_component->depth.image, vulkan_component->depth.image_memory);
                vulkan_component->depth.image_view = vulkan_component->device.createImageView(vulkan_component->depth.image, vulkan_component->depth.format, VK_IMAGE_ASPECT_DEPTH_BIT);
                vulkan_component->device.transitionImageLayout(vulkan_component->depth.image, vulkan_component->depth.format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
            }
            void RenderBase::createFrameBuffers() {
                vulkan_component->swapchain.frame_buffers.resize(vulkan_component->swapchain.image_views.size());
                for (size_t i = 0; i < vulkan_component->swapchain.image_views.size(); i++) {
                    std::array<VkImageView, 2> attachments = {
                        vulkan_component->swapchain.image_views[i],
                        vulkan_component->depth.image_view
                    };

                    VkFramebufferCreateInfo framebufferInfo = {};
                    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
                    framebufferInfo.renderPass = vulkan_component->pipeline.render_pass;
                    framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
                    framebufferInfo.pAttachments = attachments.data();
                    framebufferInfo.width = vulkan_component->swapchain.extent.width;
                    framebufferInfo.height = vulkan_component->swapchain.extent.height;
                    framebufferInfo.layers = 1;

                    if (vkCreateFramebuffer(vulkan_component->device.logical, &framebufferInfo, nullptr, &vulkan_component->swapchain.frame_buffers[i]) != VK_SUCCESS) {
                        throw std::runtime_error("failed to create framebuffer!");
                    }
                }
            }

            void RenderBase::createCommandPool()
            {
                //@COMPUTEHERE be sure to create a pool specifically for compute only
                //QueueFamilyIndices queueFamilyIndices = findQueueFamilies(vulkan_component->device.physical);

                VkCommandPoolCreateInfo poolInfo = {};
                poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
                poolInfo.queueFamilyIndex = vulkan_component->device.qFams.graphicsFamily;
                poolInfo.flags = 0; // Optional

                if (vkCreateCommandPool(vulkan_component->device.logical, &poolInfo, nullptr, &vulkan_component->command.pool) != VK_SUCCESS) {
                    throw std::runtime_error("failed to create command pool!");
                }

                //set the cmdpool pointer to the device
                //vulkan_component->device.commandPool = &commandPool;
            }

            void RenderBase::createSemaphores() {
                VkSemaphoreCreateInfo semaphoreInfo = {};
                semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

                vulkan::VK_CHECKRESULT(vkCreateSemaphore(vulkan_component->device.logical, &semaphoreInfo, nullptr, &vulkan_component->semaphores.image_available), " FAILED TO CREATE SEMAPHORE");
                vulkan::VK_CHECKRESULT(vkCreateSemaphore(vulkan_component->device.logical, &semaphoreInfo, nullptr, &vulkan_component->semaphores.render_finished), " FAILED TO CREATE SEMAPHORE");

            }

            void RenderBase::recreateSwapChain() {
                /*The disadvantage of this approach is that we need to stop all
                rendering before creating the new swap chain. It is possible to
                create a new swap chain while drawing commands on an image from
                the old swap chain are still in-flight. You need to pass the
                previous swap chain to the oldSwapChain field in the VkSwapchainCreateInfoKHR
                struct and destroy the old swap chain as soon as you've finished using it*/

                vkDeviceWaitIdle(vulkan_component->device.logical); //makes sure to tuch no resources already in use

                cleanupSwapChain();

                createSwapChain();
                createImageViews();
                createRenderPass();
                //createGraphicsPipeline();
                createDepthResources();
                createFrameBuffers();
                //createCommandBuffers();

            }
            void RenderBase::pickPhysicalDevice() {
                //Make sure there's atleasT ONE DEVICE
                uint32_t deviceCount = 0;
                vkEnumeratePhysicalDevices(vulkan_component->device.instance, &deviceCount, nullptr);
                if (deviceCount == 0)
                    throw std::runtime_error("Failed to find the GPU's with VULKAN support!");

                //array to hold all the physical devices
                std::vector<VkPhysicalDevice> devices(deviceCount);
                vkEnumeratePhysicalDevices(vulkan_component->device.instance, &deviceCount, devices.data());

                //Make sure you pick suitable devices
                for (const auto& device : devices) {
                    if (isDeviceSuitable(device)) {
                        vulkan_component->device.physical = device;
                        break;
                    }
                }
                if (vulkan_component->device.physical == VK_NULL_HANDLE) { throw std::runtime_error("failed to find a suitable GPU!"); }
            }
            VkFormat RenderBase::findDepthFormat() {
                return findSupportedFormat(
                    { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
                    VK_IMAGE_TILING_OPTIMAL,
                    VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
                );
            }
            VkFormat RenderBase::findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) {
                for (VkFormat format : candidates) {
                    VkFormatProperties props;
                    vkGetPhysicalDeviceFormatProperties(vulkan_component->device.physical, format, &props);

                    if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
                        return format;
                    else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
                        return format;
                    throw std::runtime_error("failed to find supported format!");
                }
            }

            //Make sure you pick a suitable device
            bool RenderBase::isDeviceSuitable(VkPhysicalDevice device) {
                
                //Details about basic device properties
                VkPhysicalDeviceProperties deviceProperties;
                vkGetPhysicalDeviceProperties(device, &deviceProperties);
                
                //Thanks ChatGPT for this VV
                auto printPhysicalDeviceProperties = [](VkPhysicalDeviceProperties deviceProperties) {
                    std::cout << "Device Name: " << deviceProperties.deviceName << std::endl;
                    std::cout << "Vendor ID: 0x" << std::hex << deviceProperties.vendorID << std::dec << std::endl;
                    std::cout << "Device ID: 0x" << std::hex << deviceProperties.deviceID << std::dec << std::endl;
                    std::cout << "Device Type: " << deviceProperties.deviceType << std::endl;

                    uint32_t apiVersion = deviceProperties.apiVersion;
                    uint32_t driverVersion = deviceProperties.driverVersion;

                    std::cout << "API Version: "
                            << VK_VERSION_MAJOR(apiVersion) << "."
                            << VK_VERSION_MINOR(apiVersion) << "."
                            << VK_VERSION_PATCH(apiVersion) << std::endl;

                    std::cout << "Driver Version: "
                            << VK_VERSION_MAJOR(driverVersion) << "."
                            << VK_VERSION_MINOR(driverVersion) << "."
                            << VK_VERSION_PATCH(driverVersion) << std::endl;
                };
                printPhysicalDeviceProperties(deviceProperties);


                //Details about device features
                VkPhysicalDeviceFeatures deviceFeatures;
                vkGetPhysicalDeviceFeatures(device, &deviceFeatures);


                vulkan::QueueFamilyIndices indices = findQueueFamilies(device);

                bool extensionsSupported = vulkan_component->device.checkDeviceExtensionSupport(device);
                bool swapChainAdequate = false;
                if (extensionsSupported) {
                    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
                    swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
                }
                VkPhysicalDeviceFeatures supportedFeatures;
                vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

                return indices.isComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
            }
            vulkan::QueueFamilyIndices RenderBase::findQueueFamilies(VkPhysicalDevice device) {
                vulkan::QueueFamilyIndices indices;

                //Retrieve a list of Qeueu families
                uint32_t queueFamilyCount = 0;
                vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
                std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
                vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

                //Make sure it supports graphics bit
                // 0 0 0 0, 1000 = sparse 0100 = transfer 0010 = compute 0001 = graphics
                int i = 0;
                for (const auto& queueFamily : queueFamilies) {
                    if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                        indices.graphicsFamily = i;
                    }
                    //check support for presentation
                    VkBool32 presentSupport = false;
                    vkGetPhysicalDeviceSurfaceSupportKHR(device, i, vulkan_component->swapchain.surface, &presentSupport);
                    if (queueFamily.queueCount > 0 && presentSupport) {
                        indices.presentFamily = i;
                    }

                    if (indices.isComplete()) {
                        break;
                    }

                    i++;
                }
                //possibly a transfer only queue too
                return indices;
            }
            SwapChainSupportDetails RenderBase::querySwapChainSupport(VkPhysicalDevice device) {
                SwapChainSupportDetails details;

                vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, vulkan_component->swapchain.surface, &details.capabilities);

                uint32_t formatCount;
                vkGetPhysicalDeviceSurfaceFormatsKHR(device, vulkan_component->swapchain.surface, &formatCount, nullptr);
                if (formatCount != 0) {
                    details.formats.resize(formatCount);
                    vkGetPhysicalDeviceSurfaceFormatsKHR(device, vulkan_component->swapchain.surface, &formatCount, details.formats.data());
                }

                uint32_t presentModeCount;
                vkGetPhysicalDeviceSurfacePresentModesKHR(device, vulkan_component->swapchain.surface, &presentModeCount, nullptr);
                if (presentModeCount != 0) {
                    details.presentModes.resize(presentModeCount);
                    vkGetPhysicalDeviceSurfacePresentModesKHR(device, vulkan_component->swapchain.surface, &presentModeCount, details.presentModes.data());
                }

                return details;
            }
            VkSurfaceFormatKHR RenderBase::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
                if (availableFormats.size() == 1 && availableFormats[0].format == VK_FORMAT_UNDEFINED) {
                    return { VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
                }
                else {
                    for (const auto& availableFormat : availableFormats) {
                        if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                            return availableFormat;
                        }
                    }
                }
                return availableFormats[0];
            }
            //Object that presents to the physical screen, grabs from the swap chain to present to the screen
            VkPresentModeKHR RenderBase::chooseSwapPresentMode(const std::vector <VkPresentModeKHR>availablePresentModes) {
                VkPresentModeKHR bestMode = VK_PRESENT_MODE_FIFO_KHR;
                for (const auto& availablePresentMode : availablePresentModes) {
                    if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
                        return availablePresentMode;
                    else if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR) {
                        bestMode = availablePresentMode;
                    }
                }
                //bestMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
                //bestMode = VK_PRESENT_MODE_FIFO_RELAXED_KHR;
                return bestMode;
            }
            VkExtent2D RenderBase::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
                if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
                    return capabilities.currentExtent;
                }
                else {
                    int width, height;
                    auto* g_window = g_world.get_mut<Cmp_Window>();
                    glfwGetWindowSize(g_window->window, &width, &height);

                    VkExtent2D actualExtent = {
                        static_cast<uint32_t>(width),
                        static_cast<uint32_t>(height)
                    };

                    actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
                    actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

                    return actualExtent;
                }
            }
            std::vector<const char*> RenderBase::getRequiredExtentions() {
                std::vector<const char*> extentions;

                unsigned int glfwExtensionCount = 0;
                const char** glfwExtensions;
                glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

                for (unsigned int i = 0; i < glfwExtensionCount; ++i) {
                    extentions.push_back(glfwExtensions[i]);
                }
                if (vulkan_component->device.enableValidationLayers) {
                    extentions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
                }

                return extentions;

            }


            void RenderBase::setComputeQueueFamilyIndex()
            {
                uint32_t qFamCount;
                vkGetPhysicalDeviceQueueFamilyProperties(vulkan_component->device.physical, &qFamCount, NULL); //Got the count
                std::vector<VkQueueFamilyProperties> queueFams(qFamCount);
                vkGetPhysicalDeviceQueueFamilyProperties(vulkan_component->device.physical, &qFamCount, queueFams.data());

                //first find a compute-only queue
                bool computeOnly = false;
                uint32_t i = 0;
                for (auto qfam : queueFams) {
                    if (qfam.queueCount > 0 && qfam.queueFlags == VK_QUEUE_COMPUTE_BIT) {
                        vulkan_component->device.qFams.computeFamily = i;
                        computeOnly = true;
                        break;
                    }
                    else ++i;
                }
                //If you can't find any, then do any compute fam yo
                if (!computeOnly) {
                    i = 0;
                    for (auto qfamAgn : queueFams) {
                        if (qfamAgn.queueCount > 0 && (qfamAgn.queueFlags & VK_QUEUE_COMPUTE_BIT)) {
                            vulkan_component->device.qFams.computeFamily = i;
                            break;
                        }
                        else ++i;
                    }
                    // nothing found so thro dat error y0
                    if (i == qFamCount)
                        throw std::runtime_error("couldn't find a compuete queue yo card sux");
                }
            }
        }
    }
}