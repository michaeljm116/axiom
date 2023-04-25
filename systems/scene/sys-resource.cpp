#include "pch.h"
#include "sys-resource.h"
#include <fstream>
#include "sys-log.h"
#include "sys-timer.h"
#include <filesystem>
#include <tinyxml2.h>
#include <xxhash.h>

#include <taskflow/taskflow.hpp>

namespace axiom{
    Sys_Resource::Sys_Resource(flecs::world &world_)
    {
        world = &world_;

        world->observer<Cmp_Resource, Cmp_Res_Model>()
        .event(flecs::OnSet)
        .each([this](flecs::entity e, Cmp_Resource& res, Cmp_Res_Model& d){
            this->LoadPModel(e, res, d);
        });

		world->observer<Cmp_Resource, Cmp_Res_Animations>()
        .event(flecs::OnSet)
        .each([this](flecs::entity e, Cmp_Resource& res, Cmp_Res_Animations& d){
            this->LoadPose(e, res, d);
        });

		
    }
    Sys_Resource::~Sys_Resource()
    {
    }
    bool Sys_Resource::LoadPModel(flecs::entity e, Cmp_Resource& res, Cmp_Res_Model& cmp_mod)
    {
        R_Model mod;
        auto fileName = res.file_path + "/" + res.file_name;
		std::fstream binaryio;

		binaryio.open(fileName.c_str(), std::ios::in | std::ios::binary);
		if(!binaryio.is_open()){ 
			Log(*world, LogLevel::ERROR, "Looking for Model file:" + res.file_name);
			return false;
		}

		int introLength = 0;
		int nameLength = 0;
		int numMesh = 0;
		int uniqueID = 0;
		int skeletonID = 0;
		bool skinned = false;


		//dont rly need the intro but do it anyways
		binaryio.read(reinterpret_cast<char*>(&introLength), sizeof(int));
		char c;
		for (int i = 0; i < introLength; ++i)
			binaryio.read(&c, sizeof(c));

		//Read the name;
		binaryio.read(reinterpret_cast<char*>(&nameLength), sizeof(int));
		for (int i = 0; i < nameLength; ++i) {
			binaryio.read(&c, sizeof(c));
			mod.name.push_back(c);
		}

		//Read unique ID's
		binaryio.read(reinterpret_cast<char*>(&uniqueID), sizeof(int));

		//Get num meshes;
		binaryio.read(reinterpret_cast<char*>(&numMesh), sizeof(int));

		for (int i = 0; i < numMesh; ++i) {
			R_Mesh m;
			int meshNameLength = 0;
			int numVerts = 0;
			int numFaces = 0;
			int numNodes = 0;
			int meshID = 0;

			//name
			binaryio.read(reinterpret_cast<char*>(&meshNameLength), sizeof(int));
			for (int n = 0; n < meshNameLength; ++n) {
				binaryio.read(&c, sizeof(char));
				m.name.push_back(c);
			}
			//id
			binaryio.read(reinterpret_cast<char*>(&meshID), sizeof(int));

			//nums
			binaryio.read(reinterpret_cast<char*>(&numVerts), sizeof(int));
			binaryio.read(reinterpret_cast<char*>(&numFaces), sizeof(int));
			binaryio.read(reinterpret_cast<char*>(&numNodes), sizeof(int));

			//aabbs
			binaryio.read(reinterpret_cast<char*>(&m.center), sizeof(glm::vec3));
			binaryio.read(reinterpret_cast<char*>(&m.extents), sizeof(glm::vec3));

			//vertices
			m.verts.reserve(numVerts);
			for (int v = 0; v < numVerts; ++v) {
				R_Vertex vert;
				binaryio.read(reinterpret_cast<char*>(&vert), sizeof(R_Vertex));
				//m.verts.push_back(vert);
				m.verts.emplace_back(vert);
			}
			//faces
			m.faces.reserve(numFaces);
			for (int t = 0; t < numFaces; ++t) {
				glm::ivec4 face;
				binaryio.read(reinterpret_cast<char*>(&face), sizeof(glm::ivec4));
				m.faces.emplace_back(face);
			}

			//bvh nodes
			m.bvh.reserve(numNodes);
			for (int b = 0; b < numNodes; ++b) {
				R_BVHNode node;
				binaryio.read(reinterpret_cast<char*>(&node), sizeof(R_BVHNode));
				m.bvh.emplace_back(node);
			}

			m.meshID = meshID;
			//add the model

			mod.meshes.push_back(m);
		}

		//Get number of shapes
		int numShapes = 0;
		binaryio.read(reinterpret_cast<char*>(&numShapes), sizeof(int));
		for (int i = 0; i < numShapes; ++i) {
			int shapeNameLength = 0;
			R_Shape shape;
			binaryio.read(reinterpret_cast<char*>(&shapeNameLength), sizeof(int));
			for (int n = 0; n < shapeNameLength; ++n) {
				binaryio.read(&c, sizeof(char));
				shape.name.push_back(c);
			}
			binaryio.read(reinterpret_cast<char*>(&shape.type), sizeof(int));
			binaryio.read(reinterpret_cast<char*>(&shape.center), sizeof(glm::vec3));
			binaryio.read(reinterpret_cast<char*>(&shape.extents), sizeof(glm::vec3));

			mod.shapes.push_back(shape);
		}

		int numTransforms = 0;
		binaryio.read(reinterpret_cast<char*>(&numTransforms), sizeof(int));

		////Find out of skinned or not
		//binaryio.read(reinterpret_cast<char*>(&skinned), sizeof(bool));
		////if (skinned) binaryio.read(reinterpret_cast<char*>(&skeletonID), sizeof(int));

		////get skeleton info if skinned
		//if (skinned) {
		//	//binaryio.read(reinterpret_cast<char*>(&skeletonID), sizeof(int));
		//	LoadSkeleton(binaryio, mod.name );
		//	mod.skinned = true;
		//}

		binaryio.close();
		mod.uniqueID = uniqueID;
		//mod.skeletonID = skinned ? (skeletons.end() - 1)->id : 0;
		
		//check if its triangulated
		auto triCheck = fileName.substr(fileName.length() - 5, fileName.length());
		if (triCheck == "_t") mod.triangular = true;

        //model.data = mod;
        //e.set<Cmp_Res_Model>({mod});
        e.get_mut<Cmp_Res_Model>()->data = mod;
        //e.modified<Cmp_Res_Model>();
		//auto res_cmp = e.get<Cmp_Resource>();
		return true; 
    }
    bool Sys_Resource::LoadPose(flecs::entity e, Cmp_Resource &res, Cmp_Res_Animations& cmp_anim)
    {
        auto fileName = res.file_path + "/" + res.file_name;
		assert(res.file_name.size() > 5);
		auto prefabName = res.file_name.substr(0, res.file_name.length() - 5);
		// Initialize variables
		tinyxml2::XMLDocument doc;
		tinyxml2::XMLElement* pRoot;
		tinyxml2::XMLNode* pNode;
		tinyxml2::XMLError eResult = doc.LoadFile(fileName.c_str());

		// Confirm if the thing exist
		if (eResult == tinyxml2::XML_ERROR_FILE_NOT_FOUND){ 
			Log(*world, LogLevel::ERROR, "Looking for Animation file:" + res.file_name);
			return eResult;
		}

		// Do the things
		pNode = doc.FirstChild();
		pRoot = doc.FirstChildElement("Root");

		//Iterate through the poses
		R_PoseList pl; 
		pl.name = prefabName; 
		pl.hashVal = XXH32(prefabName.c_str(), sizeof(char) * prefabName.size(), 0);
		tinyxml2::XMLElement* poseElement = pRoot->FirstChildElement("Pose");

		while (poseElement != nullptr) {
			//Get the name
			R_Pose pose;
			const char* name;
			poseElement->QueryStringAttribute("Name", &name);
			pose.name = name;
			pose.hashVal = XXH32(name, sizeof(char) * pose.name.size(), 0);
			//Iterate through the transforms
			tinyxml2::XMLElement* transElement = poseElement->FirstChildElement("Tran");

			while (transElement != nullptr) {
				int i;
				R_Sqt t;
				transElement->QueryIntAttribute("CN", &i);

				tinyxml2::XMLElement* pos = transElement->FirstChildElement("Pos");
				tinyxml2::XMLElement* rot = transElement->FirstChildElement("Rot");
				tinyxml2::XMLElement* sca = transElement->FirstChildElement("Sca");

				pos->QueryFloatAttribute("x", &t.pos.x);
				pos->QueryFloatAttribute("y", &t.pos.y);
				pos->QueryFloatAttribute("z", &t.pos.z);

				rot->QueryFloatAttribute("x", &t.rot.x);
				rot->QueryFloatAttribute("y", &t.rot.y);
				rot->QueryFloatAttribute("z", &t.rot.z);
				rot->QueryFloatAttribute("w", &t.rot.w);

				sca->QueryFloatAttribute("x", &t.sca.x);
				sca->QueryFloatAttribute("y", &t.sca.y);
				sca->QueryFloatAttribute("z", &t.sca.z);

				pose.pose.push_back(std::make_pair(i, t));
				transElement = transElement->NextSiblingElement("Tran");
			}	

			pl.poses.push_back(pose);
			poseElement = poseElement->NextSiblingElement("Pose");
		}
		
		e.get_mut<Cmp_Res_Animations>()->data = pl;
		return eResult == tinyxml2::XML_SUCCESS;
    }

    bool Sys_Resource::LoadDirectory(std::string directory)
    {
        for(const auto & p : std::filesystem::directory_iterator(directory)){
			auto extension = p.path().extension();
			const auto name = p.path().stem().string() + p.path().extension().string();
			auto e = world->entity(name.c_str());
			e.set<Cmp_Resource>({directory, name});

			if(extension == ".pm"){
				e.set<Cmp_Res_Model>({});
			}
			else if(extension == ".anim"){
				e.set<Cmp_Res_Animations>({});
			}
        }
        return true;
    }

	bool Sys_Resource::LoadDirectoryMulti(std::string directory)
	{
		tf::Taskflow taskflow;
		tf::Executor executor;

		std::vector<tf::Task> tasks;

		for (const auto &p : std::filesystem::directory_iterator(directory))
		{
			tasks.push_back(taskflow.emplace([&, p]()
			{
				auto extension = p.path().extension();
				const auto name = p.path().stem().string() + p.path().extension().string();
				auto e = world->entity(name.c_str());
				e.set<Cmp_Resource>({directory, name});

				if (extension == ".pm")
				{
					e.set<Cmp_Res_Model>({});
				}
				else if (extension == ".anim")
				{
					e.set<Cmp_Res_Animations>({});
				}
			}));
		}

		// Execute the taskflow graph
		executor.run(taskflow).wait();

		world->progress();
		return true;
	}


}