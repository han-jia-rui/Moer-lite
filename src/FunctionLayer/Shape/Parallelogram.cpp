#include "Parallelogram.h"
#include "CoreLayer/Math/Geometry.h"
#include <ResourceLayer/Factory.h>
Parallelogram::Parallelogram(const Json &json, int primID)
    : Shape(json), primitiveID_(primID),
      base(transform.toWorld(fetchRequired<Point3f>(json, "base"))),
      edge0(transform.toWorld(fetchRequired<Vector3f>(json, "edge0"))),
      edge1(transform.toWorld(fetchRequired<Vector3f>(json, "edge1"))) {
    normal = cross(edge0, edge1);

    Point3f vertices[4];
    vertices[0] = base;
    vertices[1] = vertices[0] + edge0;
    vertices[2] = vertices[1] + edge1;
    vertices[3] = vertices[0] + edge1;

    for (int i = 0; i < 4; ++i) {
        boundingBox.Expand(vertices[i]);
    }
}

bool Parallelogram::rayIntersectShape(Ray &ray, int &primID, float &u,
                                      float &v) const {
    const auto &origin = ray.origin;
    const auto &direction = ray.direction;

    auto det = -dot(direction, normal);
    if (nearZero(det)) // parallel
        return false;

    auto inv_det = 1.f / det;
    auto to = origin - base;
    auto tmp_u = dot(to, cross(direction, edge1)) * inv_det;
    auto tmp_v = dot(direction, cross(to, edge0)) * inv_det;
    if (tmp_u < .0f || tmp_v < .0f || tmp_u > 1.f || tmp_v > 1.f)
        return false;

    auto t = dot(edge1, cross(to, edge0)) * inv_det;
    if (t < ray.tNear || t > ray.tFar)
        return false;

    ray.tFar = t;
    primID = primitiveID_;
    u = tmp_u;
    v = tmp_v;

    return true;
}

void Parallelogram::fillIntersection(float distance, int primID, float u,
                                     float v,
                                     Intersection &intersection) const {
    intersection.shape = this;
    intersection.distance = distance;
    intersection.normal = normalize(normal);
    intersection.texCoord = Vector2f{u, v};
    intersection.position = base + u * edge0 + v * edge1;
    intersection.dpdu = edge0, intersection.dpdv = edge1;
    const static auto tangent = normalize(edge0);
    const static auto bitangent = normalize(cross(tangent, normal));
    intersection.tangent = tangent;
    intersection.bitangent = bitangent;
}

void Parallelogram::uniformSampleOnSurface(Vector2f sample,
                                           Intersection &result,
                                           float *pdf) const {
    const static auto area = normal.length();
    *pdf = 1.f / area;
    fillIntersection(.0f /*unused */, 0 /*unused*/, sample[0], sample[1],
                     result);
}

REGISTER_CLASS(Parallelogram, "parallelogram")
