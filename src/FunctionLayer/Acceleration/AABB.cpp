#include "AABB.h"

Point3f minP(const Point3f &p1, const Point3f &p2) {
    return Point3f{std::min(p1[0], p2[0]), std::min(p1[1], p2[1]),
                   std::min(p1[2], p2[2])};
}

Point3f maxP(const Point3f &p1, const Point3f &p2) {
    return Point3f{std::max(p1[0], p2[0]), std::max(p1[1], p2[1]),
                   std::max(p1[2], p2[2])};
}

AABB AABB::Union(const AABB &other) const {
    Point3f min = minP(other.pMin, pMin), max = maxP(other.pMax, pMax);
    return AABB{min, max};
}

void AABB::Expand(const AABB &other) {
    pMin = minP(pMin, other.pMin);
    pMax = maxP(pMax, other.pMax);
}

AABB AABB::Union(const Point3f &other) const {
    Point3f min = minP(other, pMin), max = maxP(other, pMax);
    return AABB{min, max};
}

void AABB::Expand(const Point3f &other) {
    pMin = minP(pMin, other);
    pMax = maxP(pMax, other);
}

bool AABB::Overlap(const AABB &other) const {
    for (int dim = 0; dim < 3; ++dim) {
        if (pMin[dim] > other.pMax[dim] || pMax[dim] < other.pMin[dim]) {
            return false;
        }
    }
    return true;
}

bool AABB::rayIntersect(const Ray &ray) const {
    auto tNear = ray.tNear, tFar = ray.tFar;
    for (auto i = 0; i < 3; ++i) {
        if (nearZero(ray.direction[i])) {
            return false;
        }
        auto invDir = 1.f / ray.direction[i];
        auto t0 = (pMin[i] - ray.origin[i]) * invDir,
             t1 = (pMax[i] - ray.origin[i]) * invDir;
        if (t0 > t1)
            std::swap(t0, t1);
        tNear = std::max(tNear, t0);
        tFar = std::min(tFar, t1);

        if (tNear > tFar)
            return false;
    }
    return true;
}

Point3f AABB::Center() const {
    return Point3f{(pMin[0] + pMax[0]) * .5f, (pMin[1] + pMax[1]) * .5f,
                   (pMin[2] + pMax[2]) * .5f};
}

int AABB::MaxExtent() const {
    auto span = pMax - pMin;
    if (span[0] > span[1]) {
        return span[0] > span[2] ? 0 : 2;
    } else {
        return span[1] > span[2] ? 1 : 2;
    }
}
