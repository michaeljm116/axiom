#pragma once
#include <volk.h>
#include <vector>
#include "vulkan/base.hpp"
namespace Axiom
{
    namespace Render
    {
        namespace Vulkan
        {
            /** @brief Device Queues */
            struct Queues{
                VkQueue graphics;
                VkQueue present;
                VkQueue compute;
                VkQueue copy;
            };

            struct Depth{
                VkImage image;
                VkDeviceMemory image_memory;
                VkImageView image_view;
                VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
            };

             /** @brief Swapchain*/
            struct SwapChain{
                /*struct SupportDetails
                {
                    VkSurfaceCapabilitiesKHR capabilities;
                    std::vector<VkSurfaceFormatKHR> formats;
                    std::vector<VkPresentModeKHR> present_modes;   
                } details;*/
                VkSurfaceKHR surface;
                VkSwapchainKHR get;
                VkFormat image_format;
                VkFormat color_format; //TODO THIS IS PROBABLY WORTHLESS TBH
                VkExtent2D extent;
                VkExtent2D scaled;
                
                std::vector<VkImage> images;
                std::vector<VkImageView> image_views;
                std::vector<VkFramebuffer> frame_buffers;
                uint32_t width;
                uint32_t height;
            };

            /** @brief Command Buffers and pool*/
            struct Command{
                std::vector<VkCommandBuffer> buffers;
                VkCommandPool pool;
            };

            /** @brief Semaphores*/
            struct Semaphores{
              VkSemaphore image_available;
              VkSemaphore render_finished;  
            };

            /** @brief Vulkan Pipeline Info*/               
            struct Pipeline{
                VkPipelineCache cache;
                VkRenderPass render_pass;
            };

            /** @brief Vulkan Submit info*/
            struct Submit{
                VkSubmitInfo info;
            };
        }

        struct Cmp_Vulkan
        {
            Vulkan::Device device;
            Vulkan::Semaphores semaphores;
            Vulkan::Command command;
            Vulkan::SwapChain swapchain;
            Vulkan::Depth depth;
            Vulkan::Queues queues;
            Vulkan::Pipeline pipeline;
            VkSubmitInfo submit_info;
        };
    }
}