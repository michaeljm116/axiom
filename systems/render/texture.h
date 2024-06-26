#pragma once
/*Texture class Copyright (C) by Mike Murrell 2017
honestly,,, i dont even know if i still use this class
nvm turns out i still use it
nvm turns out i still use it
*/
//#include "RenderHelplers.h"
#include "vulkan/base.hpp"
#include <assert.h>
#include "cmp-vulkan.h"

namespace Axiom 
{
	namespace Render
	{
		struct Texture
		{
		public:
			Texture();
			Texture(std::string p);
			~Texture();

			Texture(const Texture& other) = default;
			Texture& operator=(const Texture& other) = default;
			Texture(Texture&& other) noexcept = default;
			Texture& operator=(Texture&& other) noexcept = default;

			VkImage image;
			VkImageView view;
			VkImageLayout imageLayout;
			VkDeviceMemory memory;
			VkSampler sampler;
			int width, height;
			uint32_t mipLevels;
			uint32_t layerCount;
			VkDescriptorImageInfo descriptor;
			std::string path;
			VkDescriptorSet descriptor_set;
			//VulkanDevice* device;

			void destroy(VkDevice& device);
			VkResult CreateTexture(Vulkan::Device& device);
			VkResult CreateTextureMips(Vulkan::Device& device);
			void generateMipmaps(Vulkan::Device& device, VkImage image, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);
			void updateDescriptor();
		};

		struct PrPixel {
			unsigned char r = 0, g = 0, b = 0, a = 1;
			unsigned char& operator[](int i) {
				assert(i > -1 && i < 4);
				return *(&r + i);
			}
		};

		struct PrImage {
			int width, height, channels;
			std::vector<std::vector<PrPixel>> data;
			PrImage(int image_width, int image_height, int image_channels) : width(image_width), height(image_height), channels(image_channels) {
				data = std::vector(width, std::vector<PrPixel>(height));
			}
			PrImage(std::string txtr_file) {
				LoadPrImageFromTexture(txtr_file);
			}
			void LoadPrImageFromTexture(std::string txtr_file);
		};
	}
}