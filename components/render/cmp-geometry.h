/**
 * @file cmp-geometry.h
 * @author Mike Murrell (mikestoleyobike@aim.com)
 * @brief These are the structs for the geometry
 * @version 0.1
 * @date 2024-06-06
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#pragma once
#include <glm/glm.hpp>
#include "cmp-render.h"
#include "vulkan/buffer.hpp"
#include "volk.h"
#include <array>
#include <string>

namespace Axiom
{
	namespace Render
	{
		namespace Geometry
		{
			struct Vertex32
            {
				glm::vec3 p = glm::vec3();
				float u = 0.f;
				glm::vec3 n = glm::vec3();
				float v = 0.f;
				Vertex32() = default;
				Vertex32(const glm::vec3 & p, const float &u, const glm::vec3& n, const float &v) : p(p), u(u), n(n), v(v){};
				static constexpr VkVertexInputBindingDescription get_binding() {
					return VkVertexInputBindingDescription{.binding = 0, .stride = sizeof(Vertex32), .inputRate = VK_VERTEX_INPUT_RATE_VERTEX};
				}
				static constexpr std::array<VkVertexInputAttributeDescription, 4> get_attribute() {
					return {{
						{.location = 0, .binding = 0, .format = VK_FORMAT_R32G32B32_SFLOAT, .offset = offsetof(Vertex32, p)},
						{.location = 1, .binding = 0, .format = VK_FORMAT_R32_SFLOAT, .offset = offsetof(Vertex32, u)},
						{.location = 2, .binding = 0, .format = VK_FORMAT_R32G32B32_SFLOAT, .offset = offsetof(Vertex32, n)},
						{.location = 3, .binding = 0, .format = VK_FORMAT_R32_SFLOAT, .offset = offsetof(Vertex32, v)}
					}};
				}
			};
			struct Vertex48
            {
				glm::vec3 p = glm::vec3();
				float u = 0.f;
				glm::vec3 n = glm::vec3();
				float v = 0.f;
				glm::vec3 t = glm::vec3();
				int pad = 0;
				Vertex48() = default;
				Vertex48(const glm::vec3 & p, const float &u, const glm::vec3& n, const float &v) : p(p), u(u), n(n), v(v){};
				Vertex48(const glm::vec3 & p, const float &u, const glm::vec3& n, const float &v, const glm::vec3& t) : p(p), u(u), n(n), v(v), t(t){};
				static constexpr VkVertexInputBindingDescription get_binding() {
					return VkVertexInputBindingDescription{.binding = 0, .stride = sizeof(Vertex48), .inputRate = VK_VERTEX_INPUT_RATE_VERTEX};
				}
				static constexpr std::array<VkVertexInputAttributeDescription, 6> get_attribute() {
					return {{
						{.location = 0, .binding = 0, .format = VK_FORMAT_R32G32B32_SFLOAT, .offset = offsetof(Vertex48, p)},
						{.location = 1, .binding = 0, .format = VK_FORMAT_R32_SFLOAT, .offset = offsetof(Vertex48, u)},
						{.location = 2, .binding = 0, .format = VK_FORMAT_R32G32B32_SFLOAT, .offset = offsetof(Vertex48, n)},
						{.location = 3, .binding = 0, .format = VK_FORMAT_R32_SFLOAT, .offset = offsetof(Vertex48, v)},
						{.location = 4, .binding = 0, .format = VK_FORMAT_R32G32B32_SFLOAT, .offset = offsetof(Vertex48, t)},
						{.location = 5, .binding = 0, .format = VK_FORMAT_R32_SFLOAT, .offset = offsetof(Vertex48, pad)}
					}};
				}
			};

            template<typename T>
            struct Subset
            {
                std::vector<T> verts;
                std::vector<glm::ivec3> tris;
                glm::vec3 center;
                glm::vec3 extents;
                glm::uint32 mat_id;
                std::string name;

                Vulkan::VBuffer<T>           vertex_buffer;
                Vulkan::VBuffer<glm::uint32> index_buffer;
            };
            
            template<typename T>
            struct Model{
                std::vector<Subset<T>> subsets;
                std::string name;
            };
		}
	}
}