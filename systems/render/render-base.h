#pragma once
#include "vulkan/vulkan-base.hpp"
#include "vulkan/vulkan-buffer.hpp"
#include "vulkan/vulkan-initializers.hpp"
#include "render/cmp-vulkan.h"
#include "cmp-window.h"
#include "flecs-world.h"

namespace axiom
{
    namespace render
    {
        namespace base
        {
            struct SwapChainSupportDetails {
                VkSurfaceCapabilitiesKHR capabilities;
                std::vector<VkSurfaceFormatKHR> formats;
                std::vector<VkPresentModeKHR> presentModes;
            };

            struct VkDeviceInfo {
                vulkan::Device *device;
                VkQueue copyQueue;
                VkRenderPass renderPass;
                std::vector<VkFramebuffer> framebuffers;
                VkFormat colorFormat;
                VkFormat depthFormat;
                uint32_t width;
                uint32_t height;
            };

            void InitializeVulkan(Cmp_Vulkan& vulkan_component);

            class RenderBase {
            public:
                RenderBase() {};
                ~RenderBase() {

                };

                void initWindow();
                void initVulkan();
                virtual void cleanup();
                virtual void cleanupSwapChain();
                virtual void recreateSwapChain();

                static void onWindowResized(GLFWwindow* window, int width, int height) {
                    if (width == 0 || height == 0) return;

                    RenderBase* app = reinterpret_cast<RenderBase*>(glfwGetWindowUserPointer(window));
                    app->recreateSwapChain();
                }
                vulkan::Device vkDevice;

            protected:
                VkSurfaceKHR surface;

                VkImage depthImage;
                VkDeviceMemory depthImageMemory;
                VkImageView depthImageView;

                VkQueue graphicsQueue;
                VkQueue presentQueue;
                VkQueue computeQueue;
                
                VkRenderPass renderPass;
                VkPipelineCache	pipelineCache;

                VkSwapchainKHR swapChain;
                std::vector<VkImage> swapChainImages;
                VkFormat swapChainImageFormat;
                VkExtent2D swapChainExtent;

                std::vector<VkImageView> swapChainImageViews;
                std::vector<VkFramebuffer> swapChainFramebuffers;
                std::vector<VkCommandBuffer> commandBuffers;
                VkCommandPool commandPool;

                VkSemaphore imageAvailableSemaphore;
                VkSemaphore renderFinishedSemaphore;

            private:
                void createInstance();
                void createSurface();
                void createLogicalDevice();
                void createSwapChain();
                void createImageViews();
                void createRenderPass();
                void createPipelineCache();
                void createDepthResources();
                void createFrameBuffers();
                void createCommandPool();
                void createSemaphores();
                void pickPhysicalDevice();
            protected:
                VkFormat findDepthFormat();
                VkFormat findSupportedFormat(const std::vector<VkFormat>&, VkImageTiling, VkFormatFeatureFlags);

                bool isDeviceSuitable(VkPhysicalDevice);
                vulkan::QueueFamilyIndices findQueueFamilies(VkPhysicalDevice);

                SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice);
                VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>&);
                VkPresentModeKHR chooseSwapPresentMode(const std::vector <VkPresentModeKHR>);
                VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR&);
                std::vector<const char*> getRequiredExtentions();

                void setComputeQueueFamilyIndex();
            };           
        }
    }
}
