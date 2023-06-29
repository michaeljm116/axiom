#include "pch.h"
#include "sys-scene.h"
#include "sys-serialize.h"
#include "sys-log.h"
namespace Axiom
{
    namespace scene
    {
        void init()
        {
            g_world.observer<Cmp_Scene>()
            .event(flecs::OnSet)
            .each([](flecs::entity e, Cmp_Scene& s){
                load(s);
            });

        }
        void load(Cmp_Scene& scene_cmp)
        {
            // Load the file
            auto full_path = scene_cmp.path + scene_cmp.file;
            tinyxml2::XMLDocument doc;
            tinyxml2::XMLError e_result = doc.LoadFile(full_path.c_str());
            Log::Check(e_result == 0, "Loading Scene File: " + full_path + " | XMLResult:" + doc.ErrorIDToName(e_result));

            // Find the Scene node
            tinyxml2::XMLNode* p_node = doc.FirstChild();
            tinyxml2::XMLElement* p_root = doc.FirstChildElement("Root");
            Log::Check(p_root != nullptr, "Finding Root Node for " + scene_cmp.file);
            tinyxml2::XMLElement* p_scene = p_root->FirstChildElement("Scene");
            Log::Check(p_scene != nullptr, "Finding Scene Node for " + scene_cmp.file);
            p_root->FirstChildElement("Scene")->QueryIntAttribute("Num", &scene_cmp.number);

            // Load all the nodes from the first to the last
            tinyxml2::XMLElement* current_node = p_root->FirstChildElement("Node");
            while(current_node != nullptr){
                flecs::entity e = g_world.entity();
                Serialize::load_entity(current_node, e);
                current_node = current_node->NextSiblingElement();
            }
        }
    }
}