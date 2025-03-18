#pragma once
#include "Acceleration.h"

struct BVHNode {
    static constexpr int MaxLeafSize = 2;
    std::unique_ptr<BVHNode> left_node, right_node;
    int l_idx, r_idx;
    AABB bbox;
    int axis;
    bool leaf = false;

    void build(int l, int r, std::vector<std::shared_ptr<Shape>> &shapes);
    bool rayIntersect(Ray &ray, int &geomID, int &primID, float &u, float &v,
                      const std::vector<std::shared_ptr<Shape>> &shapes) const;
};

class BVH : public Acceleration {
  public:
    BVH() = default;
    void build() override;
    bool rayIntersect(Ray &ray, int &geomID, int &primID, float &u,
                      float &v) const override;

  public:
    std::unique_ptr<BVHNode> root;
};
