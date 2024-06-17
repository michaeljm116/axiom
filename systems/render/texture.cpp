#include "texture.h" 
#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#endif

namespace Axiom {
	namespace Render{
		Texture::Texture()
		{
			descriptor = {};
		}

        Texture::Texture(std::string p) : path(p)
        {
        }

        Texture::~Texture()
		{
			//destroy(device->logicalDevice);
		}

		void Texture::destroy(VkDevice& device)
		{
			vkDestroySampler(device, sampler, nullptr);
			vkDestroyImageView(device, view, nullptr);
			vkDestroyImage(device, image, nullptr);
			vkFreeMemory(device, memory, nullptr);
		}

		VkResult Texture::CreateTexture(Vulkan::Device& device)
		{
			//////////////CREATE TEXTURE IMAGE/////////////////
			//this->device = &device;
			int texWidth, texHeight, texChannels;
			stbi_uc* pixels = stbi_load(path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
			VkDeviceSize imageSize = texWidth * texHeight * 4;
			height = static_cast<uint32_t>(texHeight);
			width = static_cast<uint32_t>(texWidth);
			mipLevels = 1;

			if (!pixels) throw std::runtime_error("failed to load texture image!");

			VkBuffer stagingBuffer;
			VkDeviceMemory stagingBufferMemory;
			device.createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				stagingBuffer, stagingBufferMemory);

			void* data;
			vkMapMemory(device.logical, stagingBufferMemory, 0, imageSize, 0, &data);
			memcpy(data, pixels, static_cast<size_t>(imageSize));
			vkUnmapMemory(device.logical, stagingBufferMemory);
			stbi_image_free(pixels);

			device.createImage(texWidth, texHeight, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, image, memory, mipLevels);

			device.transitionImageLayout(image, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mipLevels);
			device.copyBufferToImage(stagingBuffer, image, width, height);
			device.transitionImageLayout(image, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, mipLevels);

			vkDestroyBuffer(device.logical, stagingBuffer, nullptr);
			vkFreeMemory(device.logical, stagingBufferMemory, nullptr);
			/////////////////////////////////////////////////////
			////////////CREATE TEXUTRE IMAGE VIEW////////////////
			view = device.createImageView(image, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT);
			/////////////////////////////////////////////////////
			/////////////CREATE TEXTURE SAMPLER//////////////////
			VkSamplerCreateInfo samplerInfo = {};
			samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
			samplerInfo.magFilter = VK_FILTER_LINEAR;
			samplerInfo.minFilter = VK_FILTER_LINEAR;

			samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

			samplerInfo.anisotropyEnable = VK_TRUE;
			samplerInfo.maxAnisotropy = 16;
			samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
			samplerInfo.unnormalizedCoordinates = VK_FALSE;

			samplerInfo.compareEnable = VK_FALSE;
			samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

			samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
			samplerInfo.mipLodBias = 0.0f;
			samplerInfo.minLod = 0.0f;
			samplerInfo.maxLod = 0.0f;

			
			//VK_CHECKRESULT(vkCreateSampler(device.logical, &samplerInfo, nullptr, &sampler), "create texture sampler!");
			vkCreateSampler(device.logical, &samplerInfo, nullptr, &sampler);

			imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			updateDescriptor();

			return VkResult();
		}

        VkResult Texture::CreateTextureMips(Vulkan::Device &device)
        {
			//////////////CREATE TEXTURE IMAGE/////////////////
			//this->device = &device;
			int texWidth, texHeight, texChannels;
			stbi_uc* pixels = stbi_load(path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
			VkDeviceSize imageSize = texWidth * texHeight * 4;
			height = static_cast<uint32_t>(texHeight);
			width = static_cast<uint32_t>(texWidth);
			mipLevels = static_cast<uint32_t>(std::floor((std::log2(std::max(texWidth, texHeight))))) + 1;

			if (!pixels) throw std::runtime_error("failed to load texture image!");

			VkBuffer stagingBuffer;
			VkDeviceMemory stagingBufferMemory;
			device.createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				stagingBuffer, stagingBufferMemory);

			void* data;
			vkMapMemory(device.logical, stagingBufferMemory, 0, imageSize, 0, &data);
			memcpy(data, pixels, static_cast<size_t>(imageSize));
			vkUnmapMemory(device.logical, stagingBufferMemory);
			stbi_image_free(pixels);

			device.createImage(texWidth, texHeight, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_SRC_BIT  | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, image, memory, mipLevels);

			device.transitionImageLayout(image, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mipLevels);
			device.copyBufferToImage(stagingBuffer, image, width, height);
			//device.transitionImageLayout(image, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, mipLevels);
			generateMipmaps(device, image, texWidth, texHeight, mipLevels);

			vkDestroyBuffer(device.logical, stagingBuffer, nullptr);
			vkFreeMemory(device.logical, stagingBufferMemory, nullptr);
			/////////////////////////////////////////////////////
			////////////CREATE TEXUTRE IMAGE VIEW////////////////
			view = device.createImageView(image, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT);
			/////////////////////////////////////////////////////
			/////////////CREATE TEXTURE SAMPLER//////////////////
			VkSamplerCreateInfo samplerInfo = {};
			samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
			samplerInfo.magFilter = VK_FILTER_LINEAR;
			samplerInfo.minFilter = VK_FILTER_LINEAR;

			samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

			samplerInfo.anisotropyEnable = VK_TRUE;
			samplerInfo.maxAnisotropy = 16;
			samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
			samplerInfo.unnormalizedCoordinates = VK_FALSE;

			samplerInfo.compareEnable = VK_FALSE;
			samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

			samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
			samplerInfo.mipLodBias = 0.0f;
			samplerInfo.minLod = 0.0f;
			samplerInfo.maxLod = 0.0f;

			
			//VK_CHECKRESULT(vkCreateSampler(device.logical, &samplerInfo, nullptr, &sampler), "create texture sampler!");
			vkCreateSampler(device.logical, &samplerInfo, nullptr, &sampler);

			imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			updateDescriptor();

			return VkResult();
        }

        void Texture::generateMipmaps(Vulkan::Device &device, VkImage image, int32_t texWidth, int32_t texHeight, uint32_t mipLevels)
        {
			VkCommandBuffer commandBuffer = device.beginSingleTimeCommands();
			VkImageMemoryBarrier barrier{
				.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
				.image = image,
				.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
				.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
				.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.subresourceRange.baseArrayLayer = 0,
				.subresourceRange.layerCount = 1,
				.subresourceRange.levelCount = 1
			};

			int32_t mip_width = texWidth;
			int32_t mip_height = texHeight;
			for(uint32_t i = 1; i < mipLevels; ++i){
				barrier.subresourceRange.baseMipLevel = i-1;
				barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
				barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
				barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
				barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

				vkCmdPipelineBarrier(commandBuffer,
					VK_PIPELINE_STAGE_TRANSFER_BIT, 
					VK_PIPELINE_STAGE_TRANSFER_BIT,
					0,
					0, nullptr,
					0, nullptr,
					1, &barrier
				);
				 
				VkImageBlit blit{
					.srcOffsets[0] = {0,0,0},
					.srcOffsets[1] = {mip_width, mip_height, 1},
					.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
					.srcSubresource.mipLevel = i - 1,
					.srcSubresource.baseArrayLayer = 0,
					.srcSubresource.layerCount = 1,
					.dstOffsets[0] = {0,0,0},
					.dstOffsets[1] = {mip_width > 1 ? mip_width / 2 : 1, mip_height > 1 ? mip_height / 2 : 1, 1},
					.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
					.dstSubresource.mipLevel = i,
					.dstSubresource.baseArrayLayer = 0,
					.dstSubresource.layerCount = 1
				};
				vkCmdBlitImage(commandBuffer,
					image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
					image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
					1, &blit, VK_FILTER_LINEAR
				);

				barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
				barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
				barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
				barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

				vkCmdPipelineBarrier(commandBuffer,
					VK_PIPELINE_STAGE_TRANSFER_BIT, 
					VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
					0,
					0, nullptr,
					0, nullptr,
					1, &barrier
				);
				if(mip_width > 1) mip_width /= 2;
				if(mip_height > 1) mip_height /= 2;
			}

			barrier.subresourceRange.baseMipLevel = mipLevels - 1;
			barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			vkCmdPipelineBarrier(commandBuffer,
				VK_PIPELINE_STAGE_TRANSFER_BIT, 
				VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
				0,
				0, nullptr,
				0, nullptr,
				1, &barrier
			);			
			device.endSingleTimeCommands(commandBuffer);
        }

        void Texture::updateDescriptor()
		{
			descriptor.sampler = sampler;
			descriptor.imageView = view;
			descriptor.imageLayout = imageLayout;
		}

		void PrImage::LoadPrImageFromTexture(std::string txtr_file)
		{
			stbi_uc* pixels = stbi_load(txtr_file.c_str(), &width, &height, &channels,0);
			data = std::vector(width, std::vector<PrPixel>(height));
			unsigned int i, j, k;
			for (k = 0; k < channels; ++k) {
				for (j = 0; j < height; ++j) {
					for (i = 0; i < width; ++i) {
						unsigned int index = k + channels * i + channels * width * j;
						data[i][j][k] = pixels[index];
					}
				}
			}
			stbi_image_free(pixels);
		}
	}
}