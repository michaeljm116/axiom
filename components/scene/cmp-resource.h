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
#include <map>

namespace Axiom{
	namespace Resource{
		struct Cmp_Directory{
			std::string assets;
		};

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

		struct Material
		{
			glm::vec3 diffuse = glm::vec3(0.0f);
			float reflective = 0.f;
			float roughness = 0.f;
			float transparency = 0.f;
			float refractiveIndex = 0.f;
			int textureID = -1;
			int uniqueID = 0;
			std::string Texture = "";
			std::string name = "";
			//ssMaterial* renderedMat;

			Material() {};
			Material(std::string n, glm::vec3 d, float rfl, float rgh, float trns, float rfr, int ti) :
				diffuse(d), reflective(rfl), roughness(rgh), transparency(trns), refractiveIndex(rfr), textureID(ti), name(n){
				uniqueID = name[0];
				for (size_t i = 1; i < name.size(); ++i) {
					uniqueID *= name[i] + name[i - 1];
				}
			};
		};

		namespace AxMaterial
        {
            template<typename T>
            struct MaterialType{
                T val;
                std::string file = "";
            };

            struct Diffuse : MaterialType<glm::vec3> {Diffuse(){val = glm::vec3();}};
            struct Specular : MaterialType<glm::vec3> {Specular(){val = glm::vec3();}};
            struct Ambient : MaterialType<glm::vec3>{Ambient(){val = glm::vec3();}};
            struct Normal : MaterialType<bool>{Normal(){val = false;}};  
            struct Albedo : MaterialType<glm::vec4>{Albedo(){val = glm::vec4();}};
            struct Metalness : MaterialType<float>{Metalness(){val = 0.f;}};
            struct Roughness : MaterialType<float>{Roughness(){val = 0.f;}};
            struct AmbientOcclusion : MaterialType<float>{AmbientOcclusion(){val = 1.f;}};
            struct Emissive : MaterialType<glm::vec4>{Emissive(){val = glm::vec4();}};

            struct Phong
            {
                Phong(){};
                Phong(Diffuse d, Specular s, Ambient a) : diffuse(d), specular(s), ambient(a){};
                Phong(Diffuse d, Specular s, Ambient a, Normal n) : diffuse(d), specular(s), ambient(a), normal(n){};
                ~Phong(){};
				
                Diffuse diffuse;
                Specular specular;
                Ambient ambient;
                Normal normal;
				std::string name = "";
				uint32_t index = 0;
            };

            struct PBR
            {
                PBR(){};
                PBR(Albedo a, Metalness m, Roughness r, Normal n) : albedo(a), metalness(m), roughness(r), normal(n){};
                PBR(Albedo a, Metalness m) : albedo(a), metalness(m){roughness = Roughness(); normal = Normal();};
                ~PBR(){}; 

                Albedo albedo;
                Metalness metalness;
                Roughness roughness;
                Normal normal;
				std::string name = "";
				uint32_t index = 0;
            };
        }

		struct Vertex {
			glm::vec3 pos;
			glm::vec3 norm;
			glm::vec2 uv;
			Vertex(const glm::vec3& p, const glm::vec3& n) : pos(p), norm(n) {};
			Vertex() {};
		};

		struct Vertex64{
			glm::vec3 pos = {};
			glm::vec3 norm = {};
			glm::vec3 tang = {};
			glm::vec2 uv = {};

			private: 
			int _pad = {};
			public:
			Vertex64(){}
			Vertex64(const glm::vec3 &p, const glm::vec3& n, const glm::vec3 &t, const glm::vec2& coord):
			pos(p), norm(n), tang(t), uv(coord){};

		};

		struct Subset{
			std::vector<Vertex64> verts;
			std::vector<glm::ivec3> tris;
			glm::vec3 center;
			glm::vec3 extents;
			glm::uint32 mat_id;
			std::string mat_name;
			std::string name;
		};

		struct TriMesh{
			std::vector<Subset> subsets;
			
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
		std::string material_type;
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

	struct Cmp_AssimpModel
	{
		std::vector<Resource::Subset> subsets;
		std::string name;
		glm::vec3 center;
		glm::vec3 extents;
	};

	struct Cmp_Assmbled
	{
		bool is_assembled = true;
	};

}