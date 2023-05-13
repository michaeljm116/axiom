#pragma once
#include "../components/scene/cmp-bvh.h"
#include <flecs-world.h>

namespace axiom{
    namespace bvh{
        static const int MAX_BVH_OBJECTS = 4;

        void Init();
        void Build();
        void Build(TreeType tt, std::vector<flecs::entity*> ops);

        BVHNode* RecursiveBuild(int start, int end, int* totalNodes, std::vector<flecs::entity*> &orderedPrims);
        BVHBounds ComputeBounds(int s, int e);
		BVHBounds ComputeCentroidBounds(int s, int e);
        int ChooseAxis(const glm::vec3& c);

        extern size_t arena_ptr;
        extern Cmp_Bvh* bvh_comp;

    }
}