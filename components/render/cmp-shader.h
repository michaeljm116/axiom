/**
 * @file cmp-shader.h
 * @author Mike Murrell (mikestoleyobike@aim.com)
 * @brief These are the structs for the shaders (memory layout is important for this)
 * @version 0.1
 * @date 2023-05-10
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#pragma once
#include <glm/glm.hpp>
#include "cmp-render.h"
#include "volk.h"
#include <array>

namespace Axiom
{
	namespace Render
	{
		namespace Shader
		{
			struct Module{
				std::vector<unsigned int> spriv;
				VkShaderModule shader_module;
			};

			enum Type{
				eVertex,
				eFragment,
				eGeometry,
				eTesselation,
				eCompute,
				eHull,
				eMesh
			};

			struct GUI {
				glm::vec2 min = glm::vec2(0.f, 0.f);
				glm::vec2 extents = glm::vec2(0.f, 0.f);
				glm::vec2 align_min = glm::vec2(0.f, 0.f);
				glm::vec2 align_ext = glm::vec2(0.f, 0.f);
				int layer = 0;
				int id = 0;
				int pad = 0;
				float alpha = 1.f;

				GUI() {};
				GUI(glm::vec2 min, glm::vec2 max, glm::vec2 aMin, glm::vec2 aExt, int l, int i) : min(min), extents(max), align_min(aMin), align_ext(aExt), layer(l), id(i) {};
			};
			//
			struct Primitive {
				glm::mat4 world = glm::mat4(); //64bytes
				glm::vec3 extents = glm::vec3(); //12bytes
				int num_children = 0; //4bytes;

				int id = 0; //4bytes
				int matId = 0; //4bytes
				int start_index = 0;
				int end_index = 0;

				Primitive() {};
				Primitive(Cmp_Primitive* pc) : world(pc->world), extents(pc->extents), num_children(pc->num_children),
					id(pc->id), matId(pc->matId), start_index(pc->start_index), end_index(pc->end_index)
				{};

			};//Total = 96bytes

			struct Vert {
				glm::vec3 pos = glm::vec3();
				float u = 0.f;
				glm::vec3 norm = glm::vec3();
				float v = 0.f;
				Vert() {};
				Vert(const glm::vec3 &p, const glm::vec3 &n, const float &u, const float &v) : pos(p), u(u), norm(n),  v(v) { };
			};

			struct V32{
				glm::vec3 p = glm::vec3();
				float u = 0.f;
				glm::vec3 n = glm::vec3();
				float v = 0.f;
				V32() = default;
				V32(const glm::vec3 & p, const float &u, const glm::vec3& n, const float &v) : p(p), u(u), n(n), v(v){};
				static constexpr VkVertexInputBindingDescription get_binding() {
					return VkVertexInputBindingDescription{.binding = 0, .stride = sizeof(V32), .inputRate = VK_VERTEX_INPUT_RATE_VERTEX};
				}
				static constexpr std::array<VkVertexInputAttributeDescription, 4> get_attribute() {
					return {{
						{.location = 0, .binding = 0, .format = VK_FORMAT_R32G32B32_SFLOAT, .offset = offsetof(V32, p)},
						{.location = 1, .binding = 0, .format = VK_FORMAT_R32_SFLOAT, .offset = offsetof(V32, u)},
						{.location = 2, .binding = 0, .format = VK_FORMAT_R32G32B32_SFLOAT, .offset = offsetof(V32, n)},
						{.location = 3, .binding = 0, .format = VK_FORMAT_R32_SFLOAT, .offset = offsetof(V32, v)}
					}};
				}
			};
			struct V48{
				glm::vec3 p = glm::vec3();
				float u = 0.f;
				glm::vec3 n = glm::vec3();
				float v = 0.f;
				glm::vec3 t = glm::vec3();
				int pad = 0;
				V48() = default;
				V48(const glm::vec3 & p, const float &u, const glm::vec3& n, const float &v) : p(p), u(u), n(n), v(v){};
				V48(const glm::vec3 & p, const float &u, const glm::vec3& n, const float &v, const glm::vec3& t) : p(p), u(u), n(n), v(v), t(t){};
				static constexpr VkVertexInputBindingDescription get_binding() {
					return VkVertexInputBindingDescription{.binding = 0, .stride = sizeof(V48), .inputRate = VK_VERTEX_INPUT_RATE_VERTEX};
				}
				static constexpr std::array<VkVertexInputAttributeDescription, 6> get_attribute() {
					return {{
						{.location = 0, .binding = 0, .format = VK_FORMAT_R32G32B32_SFLOAT, .offset = offsetof(V48, p)},
						{.location = 1, .binding = 0, .format = VK_FORMAT_R32_SFLOAT, .offset = offsetof(V48, u)},
						{.location = 2, .binding = 0, .format = VK_FORMAT_R32G32B32_SFLOAT, .offset = offsetof(V48, n)},
						{.location = 3, .binding = 0, .format = VK_FORMAT_R32_SFLOAT, .offset = offsetof(V48, v)},
						{.location = 4, .binding = 0, .format = VK_FORMAT_R32G32B32_SFLOAT, .offset = offsetof(V48, t)},
						{.location = 5, .binding = 0, .format = VK_FORMAT_R32_SFLOAT, .offset = offsetof(V48, pad)}
					}};
				}
			};

			struct UBO{                              // Compute shader uniform block object
				glm::mat4 model;
				glm::mat4 view;
				glm::mat4 proj;
			};

			struct TriangleIndex {
				glm::ivec3 v = glm::ivec3();	//12bytes
				int id = 0;			//4bytes
				TriangleIndex() {};
				TriangleIndex(glm::ivec3 v, int id) : v(v), id(id) {};
				TriangleIndex(int v0, int v1, int v2, int id) : id(id) { v[0] = v0; v[1] = v1; v[2] = v2; };
			}; //Total = 16 bytes

			struct Index {
				glm::ivec4 v = glm::ivec4(); //16bytes
				Index() {};
				Index(const glm::ivec4 &face) : v(face) {};
			};

			struct Shape {
				glm::vec3 center = glm::vec3();
				int matID = 0;
				glm::vec3 extents = glm::vec3();
				int type = 0;
				Shape() {};
				Shape(const glm::vec3& c, const glm::vec3 e, int t) : center(c), extents(e), type(t) {};
			}; //Total = 16bytes

			struct Material {
				glm::vec3 diffuse = glm::vec3();
				float reflective = 0.f;

				float roughness = 0.f;
				float transparency = 0.f;
				float refractive_index = 0.f;
				int	  texture_id = 0;
				Material() {};
				//ssMaterial(glm::vec3 d, float m, float r) { diffuse = d, metallic = m; roughness = r; };
				Material(glm::vec3 d, float rfl, float ruf, float trn, float rfr, int ti) : diffuse(d), reflective(rfl), roughness(ruf), transparency(trn), refractive_index(rfr), texture_id(ti) {};
				//ssMaterial(glm::vec3 d, float m, float r, bool t, int id) { diffuse = d; metallic = m; roughness = r; hasTexture = b; textureID = id; };
			};	//32 bytes

			struct Light {
				glm::vec3 pos = glm::vec3();
				float intensity = 0.f;
				glm::vec3 color = glm::vec3();
				int id = 0;
			};

			struct BVHNode {
				glm::vec3 upper = glm::vec3();
				int offset = 0;
				glm::vec3 lower = glm::vec3();
				int numChildren = 0;

				BVHNode(){};
				BVHNode(glm::vec3 u, glm::vec3 l, int o, int n) : upper(u), offset(o), lower(l),  numChildren(n){};
			};

			/*
			static VkVertexInputBindingDescription getVertexBindingDescription() {
				//vertex info binding
				VkVertexInputBindingDescription vibd{};
				vibd.binding = 0;
				vibd.stride = sizeof(Vert);
				vibd.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

				return vibd;
			}
			
			static std::array<VkVertexInputAttributeDescription, 4> getVertexAttributeDescriptions() {
				std::array<VkVertexInputAttributeDescription, 4> viad{};
				viad[0].binding = 0;
				viad[0].location = 0;
				viad[0].format = VK_FORMAT_R32G32B32_SFLOAT;
				viad[0].offset = offsetof(Vert, Vert::pos);

				viad[1].binding = 0;
				viad[1].location = 1;
				viad[1].format = VK_FORMAT_R32_SFLOAT;
				viad[1].offset = offsetof(Vert, Vert::u);

				viad[2].binding = 0;
				viad[2].location = 2;
				viad[2].format = VK_FORMAT_R32G32B32_SFLOAT;
				viad[2].offset = offsetof(Vert, Vert::norm);

				viad[3].binding = 0;
				viad[3].location = 3;
				viad[3].format = VK_FORMAT_R32_SFLOAT;
				viad[3].offset = offsetof(Vert, Vert::v);

				return viad;
			}

			static std::array<VkVertexInputBindingDescription, 2> getPrimitiveBindingDescriptions() 
			{
				std::array<VkVertexInputBindingDescription, 2> vibd{};

				//vertex info binding
				vibd[0].binding = 0;
				vibd[0].stride = sizeof(Vert);
				vibd[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

				//vertex info binding
				vibd[1].binding = 1;
				vibd[1].stride = sizeof(Primitive);
				vibd[1].inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;

				return vibd;
			};

			static std::array<VkVertexInputAttributeDescription, 11> getPrimitiveAttributeDescriptions() {
				std::array<VkVertexInputAttributeDescription, 11> viad{};

				viad[0].binding = 0;
				viad[0].location = 0;
				viad[0].format = VK_FORMAT_R32G32B32_SFLOAT;
				viad[0].offset = offsetof(Vert, Vert::pos);

				viad[1].binding = 0;
				viad[1].location = 1;
				viad[1].format = VK_FORMAT_R32_SFLOAT;
				viad[1].offset = offsetof(Vert, Vert::u);

				viad[2].binding = 0;
				viad[2].location = 2;
				viad[2].format = VK_FORMAT_R32G32B32_SFLOAT;
				viad[2].offset = offsetof(Vert, Vert::norm);

				viad[3].binding = 0;
				viad[3].location = 3;
				viad[3].format = VK_FORMAT_R32_SFLOAT;
				viad[3].offset = offsetof(Vert, Vert::v);

				viad[4].binding = 1;
				viad[4].location = 4;
				viad[4].format = VK_FORMAT_ASTC_4x4_SFLOAT_BLOCK_EXT;
				viad[4].offset = offsetof(Primitive, Primitive::world);

				viad[5].binding = 1;
				viad[5].location = 5;
				viad[5].format = VK_FORMAT_R32G32B32_SFLOAT;
				viad[5].offset = offsetof(Primitive, Primitive::extents);
				
				viad[6].binding = 1;
				viad[6].location = 6;
				viad[6].format = VK_FORMAT_R32_SINT;
				viad[6].offset = offsetof(Primitive, Primitive::numChildren);

				viad[7].binding = 1;
				viad[7].location = 7;
				viad[7].format = VK_FORMAT_R32_SINT;
				viad[7].offset = offsetof(Primitive, Primitive::id);

				viad[8].binding = 1;
				viad[8].location = 8;
				viad[8].format = VK_FORMAT_R32_SINT;
				viad[8].offset = offsetof(Primitive, Primitive::matId);

				viad[9].binding = 1;
				viad[9].location = 9;
				viad[9].format = VK_FORMAT_R32_SINT;
				viad[9].offset = offsetof(Primitive, Primitive::startIndex);

				viad[10].binding = 1;
				viad[10].location = 10;
				viad[10].format = VK_FORMAT_R32_SINT;
				viad[10].offset = offsetof(Primitive, Primitive::endIndex);

				return viad;
			};*/
		}
	}
}