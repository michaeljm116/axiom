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
#include "cmp-resource.h"
#include "cmp-vulkan.h"

namespace Axiom
{
	namespace Render
	{
		namespace Geometry
		{
			struct Cmp_Model_PBR
			{
				std::string name;
				uint32_t index;
				Cmp_Model_PBR(std::string n, uint32_t i) : name(n), index(i){};
			};

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

            struct Mesh48
            {
                std::vector<Vertex48> verts;
                std::vector<glm::uint32> indices;
                glm::vec3 center = {};
                glm::vec3 extents = {};
                glm::uint32 mat_id;
				std::string mat_name;
                std::string name;
                Vulkan::VBuffer<Vertex48> vertex_buffer;
                Vulkan::VBuffer<glm::uint32> index_buffer;
				Mesh48() = default;
				~Mesh48() {};
				Mesh48(const Mesh48& other)
					: verts(other.verts),
					indices(other.indices),
					center(other.center),
					extents(other.extents),
					mat_id(other.mat_id),
					mat_name(other.mat_name),
					name(other.name)
				{
				}
            
				Mesh48& operator=(const Mesh48& other) {
					if (this != &other) {
						verts = other.verts;
						indices = other.indices;
						center = other.center;
						extents = other.extents;
						mat_id = other.mat_id;
						mat_name = other.mat_name;
						name = other.name;
					}
					return *this;
				}

				
				Mesh48(const Resource::Subset& rs){
					auto num_verts = rs.verts.size();
					auto num_tris = rs.tris.size();
					auto num_indices = num_tris * 3;
					verts.reserve(num_verts);
					indices.reserve(num_indices);

					for(auto v : rs.verts){
						Vertex48 vert;
						vert.p = v.pos;
						vert.n = v.norm;
						vert.t = v.tang;
						vert.u = v.uv.x;
						vert.v = v.uv.y;
						verts.emplace_back(vert);
					}

					for(auto t : rs.tris){
						indices.emplace_back(t.x);
						indices.emplace_back(t.y);
						indices.emplace_back(t.z);
					}

					name = rs.name;
					mat_name = rs.mat_name;
				}
            };

            struct Model {
				std::vector<Mesh48> meshes;
				std::string name;

				Model() = default;  // Ensure a default constructor is available

				Model(const Model& other) : name(other.name) {
					for (const auto& mesh : other.meshes) {
						meshes.emplace_back(mesh);  // Ensure deep copy for each Mesh48
					}
				}

				Model& operator=(const Model& other) {
					if (this != &other) {
						name = other.name;
						meshes.clear();
						for (const auto& mesh : other.meshes) {
							meshes.emplace_back(mesh);
						}
					}
					return *this;
				}

				Model(const Cmp_AssimpModel* am) {
					if (am) {  // Always check for nullptr when dealing with pointers
						auto num_meshes = am->subsets.size();
						meshes.reserve(num_meshes);
						for (auto& s : am->subsets) {
							meshes.emplace_back(Mesh48(s));
						}
						name = am->name;
					}
				}
			};
		}
	}
}