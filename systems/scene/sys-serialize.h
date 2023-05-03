#pragma once
#include "../components/scene/cmp-serialize.h"
#include <tinyxml2.h>
namespace axiom
{
    namespace serialize
    {
        tinyxml2::XMLElement* save_node(flecs::entity* parent, tinyxml2::XMLDocument* doc);
        void load_entity(tinyxml2::XMLElement* start, flecs::entity* e);
    }
}