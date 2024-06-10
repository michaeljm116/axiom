#include "sys-texture.h"
#include "texture.h" 
#include <stb_image.h>

namespace Axiom
{
    namespace Render
    {
        namespace TextureManager
        {
            void initialize(){
                g_world.observer<Cmp_Texture>()
                .event(flecs::OnSet)
                .each([](flecs::entity e, Cmp_Texture& t){
                    //Cmp_Vulkan* vk = g_world.get_mut<Cmp_Vulkan>();
                    Cmp_Vulkan* vk = g_world.get_mut<Cmp_Vulkan>();
                    create_texture(t, *vk);
                });

                g_world.observer<Cmp_Texture>()
                .event(flecs::OnRemove)
                .each([](flecs::entity e, Cmp_Texture& t){
                    Cmp_Vulkan* vk = g_world.get_mut<Cmp_Vulkan>();
                    destroy(t, *vk);
                });

                g_world.observer<Resource::AxMaterial::PBR>()
                .event(flecs::OnSet)
                .each([](flecs::entity e, Resource::AxMaterial::PBR m){
                    e.set(Cmp_PBRMaterial(m.albedo.val, m.metalness.val, m.roughness.val,
                        m.albedo.file, m.metalness.file, m.roughness.file, m.normal.file));
                });

                g_world.observer<Cmp_PBRMaterial>()
                .event((flecs::OnSet))
                .each([](flecs::entity e, Cmp_PBRMaterial m){
                    Cmp_Vulkan* vk = g_world.get_mut<Cmp_Vulkan>();
                    if(m.texture_albedo.path != "") create_texture(m.texture_albedo, *vk);
                    if(m.texture_metallic.path != "") create_texture(m.texture_metallic, *vk);
                    if(m.texture_roughness.path != "") create_texture(m.texture_roughness, *vk);
                    if(m.texture_normal.path != "") create_texture(m.texture_normal, *vk);
                });
                
            }
            void destroy(Cmp_Texture& t , Cmp_Vulkan& v){
                vkDestroySampler(v.device.logical, t.sampler, nullptr);
                vkDestroyImageView(v.device.logical, t.view, nullptr);
                vkDestroyImage(v.device.logical, t.image, nullptr);
                vkFreeMemory(v.device.logical, t.memory, nullptr);
            }
            VkResult create_texture(Cmp_Texture& t, Cmp_Vulkan& v){
                //////////////CREATE TEXTURE IMAGE/////////////////
                //this->device = &device;
                int texWidth, texHeight, texChannels;
                stbi_uc* pixels = stbi_load(t.path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
                VkDeviceSize imageSize = texWidth * texHeight * 4;
                t.height = static_cast<uint32_t>(texHeight);
                t.width = static_cast<uint32_t>(texWidth);

                if (!pixels) throw std::runtime_error("failed to load texture image!");

                VkBuffer stagingBuffer;
                VkDeviceMemory stagingBufferMemory;
                v.device.createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                    stagingBuffer, stagingBufferMemory);

                void* data;
                vkMapMemory(v.device.logical, stagingBufferMemory, 0, imageSize, 0, &data);
                memcpy(data, pixels, static_cast<size_t>(imageSize));
                vkUnmapMemory(v.device.logical, stagingBufferMemory);
                stbi_image_free(pixels);

                v.device.createImage(texWidth, texHeight, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, t.image, t.memory);

                v.device.transitionImageLayout(t.image, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
                v.device.copyBufferToImage(stagingBuffer, t.image, t.width, t.height);
                v.device.transitionImageLayout(t.image, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

                vkDestroyBuffer(v.device.logical, stagingBuffer, nullptr);
                vkFreeMemory(v.device.logical, stagingBufferMemory, nullptr);
                /////////////////////////////////////////////////////
                ////////////CREATE TEXUTRE IMAGE VIEW////////////////
                t.view = v.device.createImageView(t.image, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT);
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
                vkCreateSampler(v.device.logical, &samplerInfo, nullptr, &t.sampler);

                t.image_layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                update_descriptor(t);
            }
            void update_descriptor(Cmp_Texture& t){
                t.descriptor.sampler = t.sampler;
                t.descriptor.imageView = t.view;
                t.descriptor.imageLayout = t.image_layout;
            }
        }
    }
}