#include "pch.h"
#include "sys-serialize.h"
#include "flecs-world.h"
namespace axiom
{
    namespace serialize
    {
        using namespace tinyxml2;
        tinyxml2::XMLElement *save_node(flecs::entity *parent, tinyxml2::XMLDocument *doc)
        {
            
            //Save name
            XMLElement* pNode = doc->NewElement("Node");
            pNode->SetAttribute("Name", parent->name().c_str());
            return nullptr;
        }
        void load_entity(tinyxml2::XMLElement *node)
        {
            flecs::entity e = g_world.entity();
            //Get Name
            const char* name;
            node->QueryStringAttribute("Name", &name);
            std::string unique_name = std::string(name) + "(" + std::to_string(e.id()) + ")";
            e.set_name(unique_name.c_str());
            
            //Query for children
            bool has_children;
            node->QueryBoolAttribute("hasChildren", &has_children);

            // Query for flags
            int64_t engine_flags;
            int64_t game_flags;
            node->QueryInt64Attribute("eFlags", &engine_flags);
            node->QueryInt64Attribute("gFlags", &game_flags);
            auto s = e.get_mut<Cmp_Serialize>();
            s->engine_flags = engine_flags;
            s->game_flags = game_flags;

            // Query for static or dynamic
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

            // If you have children. load dem jenkz
            
        }
    }
}