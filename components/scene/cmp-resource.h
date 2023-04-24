#pragma once
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <array>
#include <vector>
#include <string>

namespace axiom{

    struct R_Controller {
		std::array<int, 16> buttons;
		std::array<float, 6> axis;
	};
	struct R_Config {
		int numControllersConfigs;
		std::vector<R_Controller> controllerConfigs;
	};

	const float BONE_EPSILON = 0.05f;
	struct R_JointData {
		std::array<int, 4> id;
		std::array<float, 4> weights;

		R_JointData() {};
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

	struct  R_Material
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

		R_Material() {};
		R_Material(std::string n, glm::vec3 d, float rfl, float rgh, float trns, float rfr, int ti) {

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
	struct R_Vertex {
		glm::vec3 pos;
		glm::vec3 norm;
		glm::vec2 uv;
		R_Vertex(const glm::vec3& p, const glm::vec3& n) : pos(p), norm(n) {};
		R_Vertex() {};
	};

    struct R_BVHNode {
		glm::vec3 upper;
		int offset;
		glm::vec3 lower;
		int numChildren;
	};

	struct R_Mesh {
		std::vector<R_Vertex> verts;
		std::vector<glm::ivec4> faces;
		std::vector<R_BVHNode> bvh;
		std::vector<R_JointData> bones;

		glm::vec3 center;
		glm::vec3 extents;
		std::string name;
		R_Material mat;
		int matId = 0;
		int meshID;
	};

	struct R_Shape {
		std::string name;
		int type;
		glm::vec3 center;
		glm::vec3 extents;
	};
	struct R_Model {
		std::string name;
		std::vector<R_Mesh> meshes;
		std::vector<R_Shape> shapes;
		glm::vec3 center;
		glm::vec3 extents;
		int uniqueID;
		int skeletonID;
		bool skinned = false;
		bool triangular = false;
	};
    

    struct R_Sqt{
        glm::quat rot = glm::quat();
        glm::vec4 pos = glm::vec4(0);
        glm::vec4 sca = glm::vec4(1);
    }; //48 bytes

	struct R_Pose {
		std::string name;
		std::vector<std::pair<int, R_Sqt>> pose;	
		int hashVal;
	};
	struct R_PoseList {
		std::string name;
		std::vector<R_Pose> poses;
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
    struct Cmp_Resource
    {
		std::string file_path;
        std::string file_name;
    };

    struct Cmp_Res_Model
    {
        R_Model data;
    };

    struct Cmp_Res_Material
    {
        R_Material data;
    };

    struct Cmp_Res_Animations
    {
        R_PoseList data;
    };

}