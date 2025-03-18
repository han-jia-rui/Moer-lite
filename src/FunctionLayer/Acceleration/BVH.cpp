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
    int mid = (left + right) >> 1;
    std::nth_element(shapes.begin() + left, shapes.begin() + mid,
                     shapes.begin() + right,
                     [&](const auto &lhs, const auto &rhs) {
                         return lhs->getAABB().Center()[axis] <
                                rhs->getAABB().Center()[axis];
                     });
    left_node = std::make_unique<BVHNode>();
    left_node->build(left, mid, shapes);
    right_node = std::make_unique<BVHNode>();
    right_node->build(mid, right, shapes);

    bbox = left_node->bbox.Union(right_node->bbox);
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
        return left_node->rayIntersect(ray, geomID, primID, u, v, shapes) ||
               right_node->rayIntersect(ray, geomID, primID, u, v, shapes);
    } else {
        return right_node->rayIntersect(ray, geomID, primID, u, v, shapes) ||
               left_node->rayIntersect(ray, geomID, primID, u, v, shapes);
    }
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
