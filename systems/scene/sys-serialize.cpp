#include "pch.h"
#include "sys-serialize.h"
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

        void load_entitiy(tinyxml2::XMLElement *node, flecs::entity *e)
        {
            //Get Name
            const char* name;
            node->QueryStringAttribute("Name", &name);
            e->set_name(name);
            
            //Query for children
            bool has_children;
            node->QueryBoolAttribute("hasChildren", &has_children);

            // Query for flags
            int64_t engine_flags;
            int64_t game_flags;
            node->QueryInt64Attribute("eFlags", &engine_flags);
            node->QueryInt64Attribute("gFlags", &game_flags);
            auto s = e->get_mut<Cmp_Serialize>();
            s->engine_flags = engine_flags;
            s->game_flags = game_flags;

            // Query for static or dynamic
            bool dynamic;
            node->QueryBoolAttribute("Dynamic", &dynamic);
    		dynamic ?  e->set<Cmp_Dynamic>({}) : e->set<Cmp_Static>({}); 

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
                e->set<Cmp_Transform>({pos, rot, sca});
            }
        }
    }
}