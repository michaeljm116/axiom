/**
 * @file cmp-resource.h
 * @author Mike Murrell (mikestoleyobike@aim.com)
 * @brief Resource Components
 * @version 0.1
 * @date 2023-05-11
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#pragma once
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <array>
#include <vector>
#include <string>

namespace Axiom{
	namespace Resource{
		struct Controller {
			std::array<int, 16> buttons;
			std::array<float, 6> axis;
		};
		struct Config {
			int numControllersConfigs;
			std::vector<Controller> controllerConfigs;
		};

		const float BONE_EPSILON = 0.05f;
		struct JointData {
			std::array<int, 4> id;
			std::array<float, 4> weights;

			JointData() {};
			void average() {
				float total = 0;
				for (int i = 0; i < 4; ++i) {
					if (weights[i] < BONE_EPSILON)
						weights[i] = 0;
					total += weights[i];
				}
				assert(total != 0);
				for (int i = 0; i < 4; ++i) {
					weights[i] /= total;
				}
			}
		};

		struct  Material
		{
			glm::vec3 diffuse;
			float reflective;
			float roughness;
			float transparency;
			float refractiveIndex;
			int textureID = -1;
			int uniqueID = 0;
			std::string Texture;
			std::string name;
			//ssMaterial* renderedMat;

			Material() {};
			Material(std::string n, glm::vec3 d, float rfl, float rgh, float trns, float rfr, int ti) {

				name = n;			diffuse = d;
				reflective = rfl;	refractiveIndex = rfr;
				roughness = rgh;	transparency = trns;
				textureID = ti;

				uniqueID = name[0];
				for (size_t i = 1; i < name.size(); ++i) {
					uniqueID *= name[i] + name[i - 1];
				}
			}
		};
		struct Vertex {
			glm::vec3 pos;
			glm::vec3 norm;
			glm::vec2 uv;
			Vertex(const glm::vec3& p, const glm::vec3& n) : pos(p), norm(n) {};
			Vertex() {};
		};

		struct BVHNode {
			glm::vec3 upper;
			int offset;
			glm::vec3 lower;
			int numChildren;
		};

		struct Mesh {
			std::vector<Vertex> verts;
			std::vector<glm::ivec4> faces;
			std::vector<BVHNode> bvh;
			std::vector<JointData> bones;

			glm::vec3 center;
			glm::vec3 extents;
			std::string name;
			Material mat;
			int matId = 0;
			int meshID;
		};

		struct Shape {
			std::string name;
			int type;
			glm::vec3 center;
			glm::vec3 extents;
		};
		struct Model {
			std::string name;
			std::vector<Mesh> meshes;
			std::vector<Shape> shapes;
			glm::vec3 center;
			glm::vec3 extents;
			int uniqueID;
			int skeletonID;
			bool skinned = false;
			bool triangular = false;
		};
		

		struct Sqt{
			glm::quat rot = glm::quat();
			glm::vec4 pos = glm::vec4(0);
			glm::vec4 sca = glm::vec4(1);
		}; //48 bytes

		struct Pose {
			std::string name;
			std::vector<std::pair<int, Sqt>> pose;	
			int hashVal;
		};
		struct PoseList {
			std::string name;
			std::vector<Pose> poses;
			int hashVal;
		};    


		enum class Resource_Type{
			None,
			Model,
			Animation,
			Sound, 
			Texture,
			Material
		};
	};
    struct Cmp_Resource
    {
		std::string file_path;
        std::string file_name;
    };

    struct Cmp_ResModel
    {
        Resource::Model data;
    };

    struct Cmp_ResMaterial
    {
        Resource::Material data;
    };

    struct Cmp_ResAnimations
    {
        Resource::PoseList data;
    };

}