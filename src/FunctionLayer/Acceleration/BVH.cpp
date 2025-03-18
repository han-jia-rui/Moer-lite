#include "BVH.h"
#include <algorithm>
#include <memory>
#include <vector>

void BVHNode::build(int left, int right,
                    std::vector<std::shared_ptr<Shape>> &shapes) {
    l_idx = left;
    r_idx = right;

    if (right - left <= MaxLeafSize) {
        leaf = true;
        for (int i = left; i < right; ++i) {
            bbox.Expand(shapes[i]->getAABB());
        }
        return;
    }

    axis = bbox.MaxExtent();
    int step = (right - left) / MaxChildNum;
    for (int i = 0; i < MaxChildNum; ++i) {
        int l = left + i * step;
        int r = i == MaxChildNum - 1 ? right : left + (i + 1) * step;
        std::nth_element(shapes.begin() + l, shapes.begin() + r,
                         shapes.begin() + right,
                         [&](const auto &lhs, const auto &rhs) {
                             return lhs->getAABB().Center()[axis] <
                                    rhs->getAABB().Center()[axis];
                         });
        children[i] = std::make_unique<BVHNode>();
        children[i]->build(l, r, shapes);
        bbox = bbox.Union(children[i]->bbox);
    }
}

bool BVHNode::rayIntersect(
    Ray &ray, int &geomID, int &primID, float &u, float &v,
    const std::vector<std::shared_ptr<Shape>> &shapes) const {
    if (!bbox.rayIntersect(ray))
        return false;

    if (leaf) {
        for (int i = l_idx; i < r_idx; ++i) {
            if (shapes[i]->rayIntersectShape(ray, primID, u, v)) {
                geomID = shapes[i]->geometryID_;
            }
        }
        return geomID != -1;
    }

    if (ray.direction[axis] > 0) {
        for (int i = 0; i < MaxChildNum; ++i) {
            if (children[i]->rayIntersect(ray, geomID, primID, u, v, shapes))
                return true;
        }
    } else {
        for (int i = MaxChildNum - 1; i >= 0; --i) {
            if (children[i]->rayIntersect(ray, geomID, primID, u, v, shapes))
                return true;
        }
    }
    return false;
}

void BVH::build() {
    for (const auto &shape : shapes) {
        shape->initInternalAcceleration();
    }
    root = std::make_unique<BVHNode>();
    root->build(0, shapes.size(), shapes);
    boundingBox = root->bbox;
}

bool BVH::rayIntersect(Ray &ray, int &geomID, int &primID, float &u,
                       float &v) const {
    return root->rayIntersect(ray, geomID, primID, u, v, shapes);
}
