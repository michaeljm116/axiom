#include "pch.h"
#include "sys-serialize.h"
#include "flecs-world.h"
#include "../components/render/cmp-material.h"
#include "../components/render/cmp-render.h"

namespace Axiom
{
    namespace Serialize
    {
        using namespace tinyxml2;
        tinyxml2::XMLElement *save_node(flecs::entity *parent, tinyxml2::XMLDocument *doc)
        {
            
            //Save name
            XMLElement* pNode = doc->NewElement("Node");
            pNode->SetAttribute("Name", parent->name().c_str());
            return nullptr;
        }
        void load_entity(tinyxml2::XMLElement *node, flecs::entity& e)
        {
            //Get Name
            const char* name;
            node->QueryStringAttribute("Name", &name);
            std::string unique_name = std::string(name) + "(" + std::to_string(e.id()) + ")";
            e.set_name(unique_name.c_str());
            
            //Query for children
            bool has_children;
            node->QueryBoolAttribute("hasChildren", &has_children);

            //Query for flags
            int64_t engine_flags;
            int64_t game_flags;
            node->QueryInt64Attribute("eFlags", &engine_flags);
            node->QueryInt64Attribute("gFlags", &game_flags);
            auto s = e.get_mut<Cmp_Serialize>();
            s->engine_flags = engine_flags;
            s->game_flags = game_flags;

            //Query for static or dynamic
            bool dynamic;
            node->QueryBoolAttribute("Dynamic", &dynamic);
    		dynamic ?  e.add<Cmp_Dynamic>() : e.add<Cmp_Static>(); 

            //Find transform component
            glm::vec3 transPos;
            if (engine_flags & COMPONENT_TRANSFORM) {
                glm::vec3 pos;
                glm::vec3 rot;
                glm::vec3 sca;

                XMLElement* transform = node->FirstChildElement("Transform");

                XMLElement* position = transform->FirstChildElement("Position");
                position->QueryFloatAttribute("x", &pos.x);
                position->QueryFloatAttribute("y", &pos.y);
                position->QueryFloatAttribute("z", &pos.z);

                XMLElement* rotation = transform->FirstChildElement("Rotation");
                rotation->QueryFloatAttribute("x", &rot.x);
                rotation->QueryFloatAttribute("y", &rot.y);
                rotation->QueryFloatAttribute("z", &rot.z);

                XMLElement* scale = transform->FirstChildElement("Scale");
                scale->QueryFloatAttribute("x", &sca.x);
                scale->QueryFloatAttribute("y", &sca.y);
                scale->QueryFloatAttribute("z", &sca.z);

                transPos = pos;
                e.add<Cmp_Transform>();
                e.set<Cmp_Transform>({pos, rot, sca});
            }

            // if (engine_flags & COMPONENT_MATERIAL) {
            //     int matID;

            //     XMLElement* material = node->FirstChildElement("Material");
            //     material->QueryIntAttribute("ID", &matID);

            //     e.add<Render::Cmp_Material>(matID);
            //     e->addComponent(new MaterialComponent(RESOURCEMANAGER.getMaterialIndexU(matID), matID));
            // }
            if (engine_flags & COMPONENT_LIGHT) {

                XMLElement* color = node->FirstChildElement("Color");
                XMLElement* intensity = node->FirstChildElement("Intensity");
                XMLElement* id = node->FirstChildElement("ID");

                glm::vec3 c;
                float i;
                int idyo;

                color->QueryFloatAttribute("r", &c.r);
                color->QueryFloatAttribute("g", &c.g);
                color->QueryFloatAttribute("b", &c.b);

                intensity->QueryFloatAttribute("i", &i);
                id->QueryIntAttribute("id", &idyo);

                e.add<Render::Cmp_Light>();
                e.set<Render::Cmp_Light>({c,i,idyo});

                e.add<Cmp_Render>();
                e.set<Cmp_Render>({Render::RenderType::RENDER_LIGHT});

                // NodeComponent* nc = (NodeComponent*) e->getComponent<NodeComponent>();
                // nc->isParent = true;
            }
            if (engine_flags & COMPONENT_CAMERA) {
                XMLElement* ratio = node->FirstChildElement("AspectRatio");
                XMLElement* fov = node->FirstChildElement("FOV");

                float r;
                float f;

                ratio->QueryFloatAttribute("ratio", &r);
                fov->QueryFloatAttribute("fov", &f);

                e.add<Render::Cmp_Camera>();
                e.set<Render::Cmp_Camera>({r,f});

                e.add<Cmp_Render>();
                e.set<Cmp_Render>({Render::RenderType::RENDER_CAMERA});
                // NodeComponent* nc = (NodeComponent*) e->getComponent<NodeComponent>();
                // nc->isParent = true;
            }
            // if (engine_flags & COMPONENT_MODEL) {
            //     NodeComponent* nc = (NodeComponent*) e->getComponent<NodeComponent>();
            //     nc->isParent = true;
            //     int a = 4;
            // }
            //if (engine_flags & COMPONENT_MODEL) {
            //	XMLElement* Model = node->FirstChildElement("Model");
            //	int id;
            //	Model->QueryIntAttribute("ID", &id);
            //	 e->addComponent(new ModelComponent(id));
            //}
            //if (engine_flags & COMPONENT_MESH) {
            //	XMLElement* Mesh = node->FirstChildElement("Mesh");
            //	int id, ri;
            //	Mesh->QueryIntAttribute("ID", &id);
            //	Mesh->QueryIntAttribute("ResourceIndex", &ri);
            //	 e->addComponent(new MeshComponent(id, ri));
            //}
            if (engine_flags & COMPONENT_PRIMITIVE) {
                XMLElement* Object = node->FirstChildElement("Object");
                int id;
                Object->QueryIntAttribute("ID", &id);
                e.add<Render::Cmp_Primitive>();
                e.set<Render::Cmp_Primitive>({id});

                e.add<Cmp_Render>();
                e.set<Cmp_Render>({Render::RenderType::RENDER_PRIMITIVE});
            }
            if (engine_flags & COMPONENT_AABB) {
                // e->addComponent(new AABBComponent());
            }
            if (engine_flags & COMPONENT_RIGIDBODY) {
                //insertRigidBody(n);
            }
            // if (engine_flags & COMPONENT_PREFAB) {
            //     XMLElement* prefab = node->FirstChildElement("Prefab");
            //     std::string name;
            //     std::string dir;
            //     bool save; bool load; bool can_serialize;
            // }
            
            //Do the same with the children
            if(has_children){
                auto* child_element = node->FirstChildElement("Node");
                while(child_element != nullptr){
                    flecs::entity child_entity = g_world.entity().child_of(e);
                    load_entity(child_element, child_entity);
                    child_element = child_element->NextSiblingElement("Node");
                }
            }
        }
    }
}