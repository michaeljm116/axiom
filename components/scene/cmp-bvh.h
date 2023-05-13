/**
 * @file cmp-bvh.h
 * @author Mike Murrell (mikestoleyobike@aim.com)
 * @brief BVH Component
 * @version 0.1
 * @date 2023-05-11
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#pragma once
#include <GLM/glm.hpp>
#include <vector>
#include <flecs.h>

namespace axiom{
	/**
	 * @brief BVH Component, Singleton for the system
	 * @param root head of the bvh node
	 * @param num_nodes number of nodes
	 * @param ordered_prims vector of prims ordered by the BVH system
	 */
    struct Cmp_Bvh{
		bvh::BVHNode* root;
		int num_nodes;
		bool rebuild = true;
		std::vector<flecs::entity*> prims;
		std::vector<Cmp_Primitive*> prim_comps;
		bvh::SplitMethod split_method = bvh::SplitMethod::SAH;
    };
	namespace bvh{
		enum class TreeType {
			Recursive,
			HLBVH
		};
		enum class SplitMethod {
			Middle, SAH, EqualsCounts
		};

		glm::vec3 maxV(const glm::vec3& a, const glm::vec3& b){
			glm::vec3 ret;
			ret.x = std::max(a.x, b.x);
			ret.y = std::max(a.y, b.y);
			ret.z = std::max(a.z, b.z);
			return ret;
		};

		glm::vec3 minV(const glm::vec3& a, const glm::vec3& b){
			glm::vec3 ret;
			ret.x = std::min(a.x, b.x);
			ret.y = std::min(a.y, b.y);
			ret.z = std::min(a.z, b.z);
			return ret;
		};
		struct BVHBounds{
			glm::vec3 center;
			glm::vec3 extents;

			BVHBounds(glm::vec3 c, glm::vec3 e) : center(c), extents(e) {};
			BVHBounds() {};
			glm::vec3 max() {
				return center + extents;
			}
			glm::vec3 min() {
				return center - extents;
			}

			BVHBounds combine(BVHBounds b) {
				if (center.x == NAN) return b;
				//find the highest and the lowest x and y values
				glm::vec3 max = maxV(this->max(), b.max());
				glm::vec3 min = minV(this->min(), b.min());

				//center = halfway between the two, extents = max-center
				glm::vec3 c = (max + min) * 0.5f;
				glm::vec3 e = max - c;

				return BVHBounds(c, e);
			}

			BVHBounds combine(glm::vec3 c, glm::vec3 e) {
				glm::vec3 max = maxV(this->max(), (c + e));
				glm::vec3 min = minV(this->min(), (c - e));
				glm::vec3 ce = (max + min) * 0.5f;

				return BVHBounds(ce, max - ce);
			}

			float Offset(glm::vec3 c, int a) const {
				float ret = (c[a] - (center[a] - extents[a])) / (extents[a] * 2);
				return ret;
			}
			glm::vec3 Offset(glm::vec3 c) const {
				glm::vec3 ret = center;
				ret.x = Offset(c, 0);
				ret.y = Offset(c, 1);
				ret.z = Offset(c, 2);
				return ret;
			}

			float SurfaceArea() {
				glm::vec3 te = extents * 2.f;
				return 2 * (te.x * te.y + te.x * te.z + te.y * te.z);
			}
		};

		struct BVHBucket {
			int count = 0;
			BVHBounds bounds;// = BVHBounds(glm::vec3(0), glm::vec3(0));
		};

		struct BVHNode {
			BVHBounds bounds;
			BVHNode* children[2];
			int splitAxis;
			int firstPrimOffset;
			int nPrims;

			void initLeaf(int first, int n, const BVHBounds& b) {
				firstPrimOffset = first;
				nPrims = n;
				bounds = b;
				children[0] = children[1] = nullptr;
			}

			void initInterior(int axis, BVHNode* c0, BVHNode* c1) {
				children[0] = c0;
				children[1] = c1;
				//bounds = c0->bounds.union2D(c1->bounds);
				bounds = c0->bounds.combine(c1->bounds);
				splitAxis = axis;
				nPrims = 0;
			}
			void* operator new (size_t s, BVHNode* arena, size_t& curr) {
				curr += s;
				return arena + curr - s;
			}

			void* operator new (size_t s) {
				return new BVHNode();
			}
		};

		struct ssBVHNode {
			glm::vec3 upper;
			int offset;
			glm::vec3 lower;
			int numChildren;
		};

		struct LBVHTreelet {
			size_t start_index, n_primitives;
			BVHNode* build_nodes;
		};

		struct BVHPrimitive {
			BVHPrimitive() {}
			BVHPrimitive(size_t prim_ind, const BVHBounds &_b) : primitive_index(prim_ind), bounds(_b) {}
			size_t primitive_index;
			BVHBounds bounds;
			glm::vec3 Centroid() const { return bounds.center; }
		};
	}
}