#pragma once

#include <algorithm>

#include "aabb.hpp"
#include "surface.hpp"

class BVHNode
{
public:
    AABB aabb;
    union {
        BVHNode* children[2];
        Surface* surface;
    };
    bool isLeaf;

    float sah(const std::vector<float>& areas0, const std::vector<float>& areas1,
            size_t subset_offset, size_t subset_size, int i)
    {
        return i * areas0[subset_offset + i - 1] + (subset_size - i) * areas1[subset_offset + i];
    }

    BVHNode()
    {
    }

    ~BVHNode()
    {
        if (!isLeaf) {
            delete children[0];
            delete children[1];
        }
    }

    void build(const std::vector<std::unique_ptr<Surface>>& surfaces,
            const std::vector<AABB>& aabbs,
            std::vector<unsigned int>& subset,
            std::vector<float>& areas0, std::vector<float>& areas1,
            size_t I, size_t N)
    {
        if (N == 1) {
            aabb = aabbs[subset[I]];
            surface = surfaces[subset[I]].get();
            isLeaf = true;
        } else {
            // find the AABB for the complete subset, determine its longest
            // axis, and sort the hitables along that axis
            aabb = aabbs[subset[I]];
            for (size_t i = 1; i < N; i++)
                aabb = merge(aabb, aabbs[subset[I + i]]);
            AABBSorter sorter(aabbs, aabb.longestAxis());
            std::sort(subset.begin() + I, subset.begin() + I + N, sorter);
            // The splitIndex is the index of the first hitable that goes to
            // _children[1]; all before that go to _children[0]. It must be in
            // [1, n-1] so that both children get at least one hitable.
            AABB box0 = aabbs[subset[I]];
            areas0[I + 0] = box0.surfaceArea();
            for (size_t i = 1; i < N - 1; i++) {
                box0 = merge(box0, aabbs[subset[I + i]]);
                areas0[I + i] = box0.surfaceArea();
            }
            AABB box1 = aabbs[subset[I + N - 1]];
            areas1[I + N - 1] = box1.surfaceArea();
            for (int i = N - 2; i > 0; i--) {
                box1 = merge(box1, aabbs[subset[I + i]]);
                areas1[I + i] = box1.surfaceArea();
            }
            // find the optimal splitIndex according to SAH
            int splitIndex = 1;
            float minSAH = sah(areas0, areas1, I, N, 1);
            for (size_t i = 2; i < N; i++) {
                float SAH = sah(areas0, areas1, I, N, i);
                if (SAH < minSAH) {
                    minSAH = SAH;
                    splitIndex = i;
                }
            }
            // divide surface set into two new sets
            size_t I0 = I;
            size_t N0 = splitIndex;
            size_t I1 = I + splitIndex;
            size_t N1 = N - N0;
            children[0] = new BVHNode;
            children[1] = new BVHNode;
            children[0]->build(surfaces, aabbs, subset, areas0, areas1, I0, N0);
            children[1]->build(surfaces, aabbs, subset, areas0, areas1, I1, N1);
            isLeaf = false;
        }
    }

    static const BVHNode* buildBVH(const std::vector<std::unique_ptr<Surface>>& surfaces, float t0, float t1)
    {
        fprintf(stderr, "Building bounding volume hierarchy for %zu surfaces for %.3fs-%.3fs... ",
                surfaces.size(), t0, t1);
        BVHNode* rootNode = new BVHNode;
        std::vector<AABB> aabbs(surfaces.size());
        std::vector<unsigned int> subset(surfaces.size());
        for (size_t i = 0; i < surfaces.size(); i++) {
            aabbs[i] = surfaces[i]->aabb(t0, t1);
            subset[i] = i;
        }
        std::vector<float> areas0(surfaces.size());
        std::vector<float> areas1(surfaces.size());
        rootNode->build(surfaces, aabbs, subset, areas0, areas1, 0, subset.size());
        fprintf(stderr, "done\n");
        return rootNode;
    }

    void measure(size_t& nodeCount, size_t& maxDepth, size_t depth = 1) const
    {
        size_t nc1 = 0;
        size_t md1 = depth;
        size_t nc2 = 0;
        size_t md2 = depth;
        if (!isLeaf) {
            children[0]->measure(nc1, md1, depth + 1);
            children[1]->measure(nc2, md2, depth + 1);
        }
        nodeCount = 1 + nc1 + nc2;
        maxDepth = std::max(md1, md2);
    }
};

class BVHNodeLinear
{
public:
    AABB aabb;
    union {
        size_t child2IndexMulTwoPlusOne;
        const Surface* surface;
    };
};

class BVHTreeLinear : public Surface
{
public:
    static const size_t maxTreeDepth = 128;
    std::vector<BVHNodeLinear> nodes;

    BVHTreeLinear()
    {
        static_assert(sizeof(BVHNodeLinear) == 32);
    }

    size_t flattenBVH(const BVHNode* node, size_t* offset)
    {
        size_t myOffset = (*offset)++;
        BVHNodeLinear& linearNode = nodes[myOffset];
        linearNode.aabb = node->aabb;
        if (node->isLeaf) {
            linearNode.surface = node->surface;
        } else {
            flattenBVH(node->children[0], offset);
            size_t child2Index = flattenBVH(node->children[1], offset);
            linearNode.child2IndexMulTwoPlusOne = child2Index * 2 + 1;
        }
        return myOffset;
    }

    void build(const std::vector<std::unique_ptr<Surface>>& surfaces, float t0, float t1)
    {
        const BVHNode* root = BVHNode::buildBVH(surfaces, t0, t1);
        size_t nodeCount, maxDepth;
        root->measure(nodeCount, maxDepth);
        if (maxDepth > maxTreeDepth) {
            fprintf(stderr, "Too many levels\n");
            abort();
        }
        fprintf(stderr, "Linearizing bounding volume hierarchy with %zu nodes on %zu levels... ", nodeCount, maxDepth);
        nodes.resize(nodeCount);
        size_t offset = 0;
        flattenBVH(root, &offset);
        delete root;
        fprintf(stderr, "done.\n");
    }

    virtual AABB aabb(float /* t0 */, float /* t1 */) const override
    {
        return nodes[0].aabb;
    }

    virtual HitRecord hit(const Ray& ray, float amin, float amax) const override
    {
        HitRecord hr;
        size_t toVisitOffset = 0;
        size_t currentNodeIndex = 0;
        size_t nodesToVisit[maxTreeDepth];
        for (;;) {
            const BVHNodeLinear& node = nodes[currentNodeIndex];
            if (node.aabb.hit(ray, amin, amax)) {
                if (node.child2IndexMulTwoPlusOne % 2 == 0) {
                    HitRecord currentHr = node.surface->hit(ray, amin, amax);
                    if (currentHr.haveHit) {
                        hr = currentHr;
                        amax = hr.a;
                    }
                    if (toVisitOffset == 0)
                        break;
                    currentNodeIndex = nodesToVisit[--toVisitOffset];
                } else {
                    nodesToVisit[toVisitOffset++] = node.child2IndexMulTwoPlusOne / 2; // child 2
                    currentNodeIndex++; // child 1
                }
            } else {
                if (toVisitOffset == 0)
                    break;
                currentNodeIndex = nodesToVisit[--toVisitOffset];
            }
        }

        return hr;
    }
};
