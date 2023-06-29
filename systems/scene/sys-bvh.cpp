#include "pch.h"
#include "sys-bvh.h"
#include "../components/render/cmp-render.h"
#include <algorithm>

namespace Axiom{
    namespace Bvh{
        size_t arena_ptr = 0;
        Cmp_Bvh* bvh_comp;

        void Init()
        {

            bvh_comp = g_world.get_mut<Cmp_Bvh>();
        }
        void Build()
        {
        }
        void Build(TreeType tt, std::vector<flecs::entity *> ops)
        {
            //Find the number of primitives
            auto num_prims = flecs::query<Cmp_Primitive>().count();
            
            //Prepare to build
            bvh_comp->num_nodes = 0;

            //Build the bvh and move to the singleton component
            bvh_comp->root = RecursiveBuild(0, num_prims, &bvh_comp->num_nodes, ops);
            bvh_comp->prims = std::move(ops);
        }

        BVHNode* RecursiveBuild(int start, int end, int *totalNodes, std::vector<flecs::entity *> &orderedPrims)
        {
            *totalNodes += 1;
            BVHNode* node = new(bvh_comp->root, arena_ptr) BVHNode(); // new BVHNode(root, arena_ptr);//(new BVHNode);// std::make_shared<BVHNode>();
            BVHBounds bounds = ComputeBounds(start, end);

            //Check if leaf
            int numPrims = end - start;
            int prevOrdered = orderedPrims.size();
            if (numPrims < MAX_BVH_OBJECTS) { //create leaf
                for (int i = start; i < end; ++i)
                    orderedPrims.emplace_back(bvh_comp->prims[i]);
                node->initLeaf(prevOrdered, numPrims, bounds);
            }
            //Not a leaf, create a new node
            else {
                BVHBounds centroid = ComputeCentroidBounds(start, end);
                int axis = ChooseAxis(centroid.center);
                int mid = (start + end) >> 1;

                //edgecase
                if (centroid.max()[axis] == centroid.min()[axis]) {
                    for (int i = start; i < end; ++i)
                        orderedPrims.emplace_back(bvh_comp->prims[i]);
                    node->initLeaf(prevOrdered, numPrims, bounds);
                    return node;
                }
                else {
                    switch (bvh_comp->split_method) {
                    case SplitMethod::Middle: {
                        flecs::entity **midPtr = std::partition(&bvh_comp->prims[start], &bvh_comp->prims[end - 1] + 1, [axis, centroid](flecs::entity * a) {
                            //return ptm->get(*a)->center()[axis] < centroid.center[axis];
                            return a->get_mut<Cmp_Primitive>()->center()[axis] < centroid.center[axis];
                        });
                        mid = midPtr - &bvh_comp->prims[0];
                    }
                    case SplitMethod::EqualsCounts: {
                        std::nth_element(&bvh_comp->prims[start], &bvh_comp->prims[mid], &bvh_comp->prims[end - 1] + 1, [axis](flecs::entity* a, flecs::entity* b) {
                            return a->get_mut<Cmp_Primitive>()->center()[axis] < b->get_mut<Cmp_Primitive>()->center()[axis];
                        });
                    }
                    case SplitMethod::SAH: {
                        if (numPrims <= MAX_BVH_OBJECTS) {
                            mid = (start + end) >> 1;
                            std::nth_element(&bvh_comp->prims[start], &bvh_comp->prims[mid], &bvh_comp->prims[end - 1] + 1, [axis](flecs::entity* a, flecs::entity* b) {
                                return a->get_mut<Cmp_Primitive>()->center()[axis] < b->get_mut<Cmp_Primitive>()->center()[axis];
                            });
                        }
                        else {
                            //initialize the buckets
                            constexpr int numBuckets = 12;
                            BVHBucket buckets[numBuckets];
                            for (int i = start; i < end; ++i) {
                                //PrimitiveComponent* pc = ptm->get(*bvh_comp->prims[i]);
                                auto* pc = bvh_comp->prims[i]->get_mut<Cmp_Primitive>();
                                BVHBounds tempBounds = BVHBounds(pc->center(), pc->aabbExtents);
                                int b = numBuckets * centroid.Offset(pc->center(), axis);
                                if (b == numBuckets) b--;
                                buckets[b].count++;
                                buckets[b].bounds = buckets[b].bounds.combine(tempBounds);
                            }

                            constexpr int nb = numBuckets - 1;
                            float cost[nb];
                            for (int i = 0; i < nb; ++i) {
                                BVHBounds b0, b1;
                                int c0 = 0, c1 = 0;

                                for (int j = 0; j <= i; ++j) {
                                    b0 = b0.combine(buckets[j].bounds);
                                    c0 += buckets[j].count;
                                }
                                for (int j = i + 1; j < numBuckets; ++j) {
                                    b1 = b1.combine(buckets[j].bounds);
                                    c1 += buckets[j].count;
                                }
                                cost[i] = .125f + (c0 * b0.SurfaceArea() + c1 * b1.SurfaceArea()) / bounds.SurfaceArea();
                            }

                            float minCost = cost[0];
                            int minCostSplitBucket = 0;
                            for (int i = 0; i < nb; ++i) {
                                if (cost[i] < minCost) {
                                    minCost = cost[i];
                                    minCostSplitBucket = i;
                                }
                            }
                            float leafCost = numPrims;
                            if (numPrims > MAX_BVH_OBJECTS || minCost < leafCost) {
                                flecs::entity **midPtr = std::partition(&bvh_comp->prims[start], &bvh_comp->prims[end - 1] + 1, [axis, centroid, minCostSplitBucket, numBuckets](flecs::entity * a) {
                                    int b = (numBuckets)* centroid.Offset(a->get_mut<Cmp_Primitive>()->center(), axis);
                                    if (b == numBuckets) b = numBuckets - 1;
                                    return b <= minCostSplitBucket;

                                });
                                mid = midPtr - &bvh_comp->prims[0];
                                if (mid != start && mid != end)
                                    break;
                                else {
                                    for (int i = start; i < end; ++i)
                                        orderedPrims.emplace_back(bvh_comp->prims[i]);
                                    node->initLeaf(prevOrdered, numPrims, bounds);
                                    return node;
                                }

                            }
                            else { //create leaf
                                for (int i = start; i < end; ++i)
                                    orderedPrims.emplace_back(bvh_comp->prims[i]);
                                node->initLeaf(prevOrdered, numPrims, bounds);
                                return node;
                            }
                        }
                    }
                    default:
                        break;
                    }

                    node->initInterior(axis, RecursiveBuild(start, mid, totalNodes, orderedPrims), RecursiveBuild(mid, end, totalNodes, orderedPrims));
                }
            }
            return node;
        }
        BVHBounds ComputeBounds(int s, int e)
        {
            return BVHBounds();
        }
        BVHBounds ComputeCentroidBounds(int s, int e){
            //make an aabb of the entire scene basically
            //find the minimum x and maximum x and bounds = max-min/2 = center
            glm::vec3 min(FLT_MAX);
            glm::vec3 max(-FLT_MAX);
            for (int i = s; i < e; ++i) {
                auto* pc = bvh_comp->prims[i]->get_mut<Cmp_Primitive>();
                min = minV(min, pc->center() - pc->aabbExtents);
                max = maxV(max, pc->center() + pc->aabbExtents);
            }
            glm::vec3 c = (max + min) * 0.5f;
            glm::vec3 ex = max - c;
            return BVHBounds(c, ex);
        }
        int ChooseAxis(const glm::vec3 &c){
            if ((c.x > c.y) && (c.x > c.z)) return 0;
            else if ((c.y > c.x) && (c.y > c.z)) return 1;
            else return 2;
        }
    }
}