#pragma once
#include <volk.h>
#include <vector>
#include <array>
#include "vulkan/base.hpp"
namespace Axiom
{
    namespace Render
    {
        static constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 3;
        namespace Vulkan
        {
            /** @brief Device Queues */
            struct Queues{
                VkQueue graphics = VK_NULL_HANDLE;
                VkQueue present = VK_NULL_HANDLE;
                VkQueue compute = VK_NULL_HANDLE;
                VkQueue copy = VK_NULL_HANDLE;
            };

            struct Depth{
                VkImage image = VK_NULL_HANDLE;
                VkDeviceMemory image_memory = VK_NULL_HANDLE;
                VkImageView image_view = VK_NULL_HANDLE;
                VkFormat format = VK_FORMAT_UNDEFINED;
            };

             /** @brief Swapchain*/
            struct SwapChain{
                /*struct SupportDetails
                {
                    VkSurfaceCapabilitiesKHR capabilities;
                    std::vector<VkSurfaceFormatKHR> formats;
                    std::vector<VkPresentModeKHR> present_modes;   
                } details;*/
                VkSurfaceKHR surface = VK_NULL_HANDLE;
                VkSwapchainKHR get = VK_NULL_HANDLE;
                VkFormat image_format = VkFormat::VK_FORMAT_UNDEFINED;
                VkFormat color_format = VkFormat::VK_FORMAT_UNDEFINED; //TODO THIS IS PROBABLY WORTHLESS TBH
                VkExtent2D extent = VkExtent2D(0,0);
                VkExtent2D scaled = VkExtent2D(0,0);
                
                std::vector<VkImage> images = {};
                std::vector<VkImageView> image_views = {};
                std::vector<VkFramebuffer> frame_buffers = {};
                uint32_t width = 1920;
                uint32_t height = 1080;
            };

            /** @brief Command Buffers and pool*/
            struct Command{
                std::vector<VkCommandBuffer> buffers = {};
                VkCommandPool pool = VK_NULL_HANDLE;
            };

            /** @brief Semaphores*/
            struct Semaphores{
              VkSemaphore image_available = VK_NULL_HANDLE;
              VkSemaphore render_finished = VK_NULL_HANDLE;  
              VkFence in_flight_fence = VK_NULL_HANDLE;

              std::array<VkSemaphore, MAX_FRAMES_IN_FLIGHT> render_ready = {};
              std::array<VkSemaphore, MAX_FRAMES_IN_FLIGHT> frame_presented = {};
              std::array<VkFence, MAX_FRAMES_IN_FLIGHT> presentation_fence = {};
            };

            /** @brief Vulkan Pipeline Info*/               
            struct Pipeline{
                VkPipelineCache cache = VK_NULL_HANDLE;
                VkRenderPass render_pass = VK_NULL_HANDLE;
            };

            /** @brief Vulkan Submit info*/
            struct Submit{
                VkSubmitInfo info;
            };
        }

        struct Cmp_Vulkan
        {
            Vulkan::Device device = Vulkan::Device();
            Vulkan::Semaphores semaphores = Vulkan::Semaphores();
            Vulkan::Command command = Vulkan::Command();
            Vulkan::SwapChain swapchain = Vulkan::SwapChain();
            Vulkan::Depth depth = Vulkan::Depth();
            Vulkan::Queues queues = Vulkan::Queues();
            Vulkan::Pipeline pipeline = Vulkan::Pipeline();
            VkSubmitInfo submit_info = VkSubmitInfo();
        };
    }
}