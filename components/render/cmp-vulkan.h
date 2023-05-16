#pragma once
#include <volk.h>
#include <vector>
namespace axiom
{
    namespace render
    {
        namespace vulkan
        {
            struct Queues{
                VkQueue graphics;
                VkQueue present;
                VkQueue compute;
            };

            struct SwapChain{
                struct SupportDetails
                {
                    VkSurfaceCapabilitiesKHR capabilities;
                    std::vector<VkSurfaceFormatKHR> formats;
                    std::vector<VkPresentModeKHR> present_modes;   
                } details;
                VkSwapchainKHR get;
                VkFormat image_format;
                VkExtent2D extent;
                
                std::vector<VkImage> images;
                std::vector<VkImageView> image_views;
                std::vector<VkFramebuffer> frame_buffers;
            };

            struct Command{
                std::vector<VkCommandBuffer> buffers;
                VkCommandPool pool;
            };

            struct Semaphores{
              VkSemaphore image_available;
              VkSemaphore render_finished;  
            }

        }
        struct Cmp_Vulkan{

        }


    }
}