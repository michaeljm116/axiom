#include "pch.h"
#include "sys-resource.h"
#include <fstream>
#include "sys-log.h"
#include <filesystem>
namespace axiom{
    Sys_Resource::Sys_Resource(flecs::world &world_)
    {
        world = &world_;

        world->observer<Cmp_Resource>()
        .event(flecs::OnSet)
        .each([this](flecs::entity e, Cmp_Resource& res){
            this->LoadPModel(e, res);
        });
    }
    Sys_Resource::~Sys_Resource()
    {
    }
    bool Sys_Resource::LoadPModel(flecs::entity e, Cmp_Resource& res)
    {
        R_Model mod;
        auto fileName = res.file_path + "/" + res.file_name;
		std::fstream binaryio;

		binaryio.open(fileName.c_str(), std::ios::in | std::ios::binary);
		Check(*world, binaryio.is_open(), "Opening File: " + res.file_name);
		if(!binaryio.is_open()) return false;

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
        e.set<Cmp_Res_Model>({mod});
        //e.get_mut<Cmp_Res_Model>()->data = mod;
        //e.modified<Cmp_Res_Model>();
		//auto res_cmp = e.get<Cmp_Resource>();
        Check(*world, true, "Loading Model: " + res.file_name);
		return true; 
    }
    bool Sys_Resource::LoadDirectory(std::string directory)
    {
        for(const auto & p : std::filesystem::directory_iterator(directory)){
			auto extension = p.path().extension();
			if(extension == ".pm"){
				const auto name = p.path().stem().string() + p.path().extension().string();
				auto e = world->entity(name.c_str());
				e.set<Cmp_Resource>({directory, name});
				e.modified<Cmp_Resource>();
			}
        }
        world->progress();
        return true;
    }
}