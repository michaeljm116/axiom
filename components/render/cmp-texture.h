#pragma once
#include "vulkan/base.hpp"
#include "cmp-vulkan.h"

namespace Axiom
{
    namespace Render
    {
        struct Cmp_Texture
        {
            VkImage image = {};
			VkImageView view = {};
			VkImageLayout image_layout = {};
			VkDeviceMemory memory = {};
			VkSampler sampler = {};
			int width = 128, height = 128;
			uint32_t mip_levels = 1;
			uint32_t layer_count = 1;
			VkDescriptorImageInfo descriptor = {};
			std::string path = "";
			VkDescriptorSet descriptor_set = {};

            Cmp_Texture(std::string p) : path(p){};
        };
    }
}