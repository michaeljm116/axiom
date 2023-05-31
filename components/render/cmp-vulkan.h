#pragma once
#include <volk.h>
#include <vector>
#include "vulkan/vulkan-base.hpp"
namespace axiom
{
    namespace render
    {
        namespace vulkan
        {
            /** @brief Device Queues */
            struct Queues{
                VkQueue graphics;
                VkQueue present;
                VkQueue compute;
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
                VkExtent2D extent;
                
                std::vector<VkImage> images;
                std::vector<VkImageView> image_views;
                std::vector<VkFramebuffer> frame_buffers;
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

            /** @brief Vulkan Device Info*/
            struct DeviceInfo{
                vulkan::Device* device;
                VkQueue copy_queue;
                VkRenderPass render_pass;
                std::vector<VkFramebuffer> frame_buffers;
                VkFormat color_format;
                VkFormat depth_format;
                uint32_t width;
                uint32_t heigt;
                
            };
        }

        struct Cmp_Vulkan
        {
            vulkan::Device device;
            vulkan::Semaphores semaphores;
            vulkan::Command command;
            vulkan::SwapChain swapchain;
            vulkan::Queues queues;
        };
    }
}