/**
 * @file cmp-render.h
 * @author Mike Murrell (mikestoleyobike@aim.com)
 * @brief Components for rendering
 * @version 0.1
 * @date 2023-05-10
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#pragma once
#include <glm/glm.hpp>
#include <vector>

namespace Axiom
{
	namespace Render
	{
		enum RenderType {
			RENDER_NONE = 0x00,
			RENDER_MATERIAL = 0x01,
			RENDER_PRIMITIVE = 0x02,
			RENDER_LIGHT = 0x04,
			RENDER_GUI = 0x08,
			RENDER_GUINUM = 0x10,
			RENDER_CAMERA = 0x20
		};

		enum RendererType {
			kComputeRaytracer,
			kHardwareRaytracer,
			kComputeRasterizer,
			kHardwareRasterizer
		};

		struct Cmp_Light
		{
			glm::vec3 color;
			float intensity;
			int id = 0;
		};

		struct Cmp_Primitive
		{
			glm::mat4 world; //64bytes
			glm::vec3 extents; //12bytes
			glm::vec3 aabb_extents; //12bytes
			int num_children = 0; //4bytes;
			
			int id; //4bytes
			int matId; //4bytes
			int start_index = 0; //4bytes
			int end_index = 0; //4bytes

			//total = 108bytes

			inline glm::vec3 center() { return glm::vec3(world[3].x, world[3].y, world[3].z); };  
		};

		struct Cmp_Sphere {
			float radius;
			int index;

			Cmp_Sphere() {};
			Cmp_Sphere(float r) { radius = r; };
		};

		struct Cmp_Box {
			glm::vec3 center;
			glm::vec3 extents;
			int index;

			Cmp_Box() {};
			Cmp_Box(glm::vec3 c, glm::vec3 e) : center(c), extents(e) {};
		};

		struct Cmp_Cylinder {
			glm::vec3 top;
			glm::vec3 bottom; 
			float radius;
			int index;

			Cmp_Cylinder() {};
			Cmp_Cylinder(glm::vec3 t, glm::vec3 b, float r) : top(t), bottom(b), radius(r) {};
		};

		struct Cmp_Plane {
			glm::vec3 normal;
			float distance;
			int index;

			Cmp_Plane() {};
			Cmp_Plane(glm::vec3 n, float d) :normal(n), distance(d) {};
		};

		struct Cmp_Mesh {
			int index;
			int model_id;
			int resource_index;
			int unique_id;

			Cmp_Mesh() {};
			Cmp_Mesh(int si) { index = si; };
			Cmp_Mesh(int id, int ri) : model_id(id), resource_index(ri) {};
		};

		struct Cmp_Model {
			int index;
			int unique_id;

			Cmp_Model() {};
			Cmp_Model(int n) { unique_id = n; };
			Cmp_Model(int n, int id) { index = n; unique_id = id; };
		};

		enum class SelectableState {
			Unselected, 
			Released, 
			Held, 
			Pressed
		};
		struct Cmp_Selectable {
			SelectableState state;
			bool active = false;
			bool reset = false;
		};

		struct Cmp_GUI {
			glm::vec2 min;
			glm::vec2 extents;
			glm::vec2 align_min;
			glm::vec2 align_ext;
			int layer;
			int id;
			int ref;
			float alpha = 0.f;
			bool update = true;
			//bool visible;
			Cmp_GUI() {};
			Cmp_GUI(glm::vec2 m, glm::vec2 e, glm::vec2 amin, glm::vec2 ae, int l, int i, float a) :
				min(m), extents(e), align_min(amin), align_ext(ae), layer(l), id(i), alpha(a) {};
		};
		struct Cmp_GUINumber : Cmp_GUI {
			int number = 0;
			int highest_active_digit_index = 0;
			std::vector<int> shaderReferences;
			Cmp_GUINumber() {};
			Cmp_GUINumber(glm::vec2 m, glm::vec2 e, int n) { min = m; extents = e; number = n; align_min = glm::vec2(0.0f, 0.0f); align_ext = glm::vec2(0.1f, 1.f); layer = 0; id = 0; };
			Cmp_GUINumber(glm::vec2 m, glm::vec2 e, int n, float a) { min = m; extents = e; number = n; align_min = glm::vec2(0.0f, 0.0f); align_ext = glm::vec2(0.1f, 1.f); layer = 0; id = 0; alpha = a; };
		};

	};

	struct Cmp_Render {
		Render::RenderType type;
		Render::RendererType renderer = Render::kComputeRaytracer;
		Cmp_Render() {};
		Cmp_Render(Render::RenderType t) : type(t) {};
	};
} // namespace axio