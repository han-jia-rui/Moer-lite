#include "Triangle.h"
#include "CoreLayer/Math/Constant.h"
#include "CoreLayer/Math/Geometry.h"
#include <FunctionLayer/Acceleration/Linear.h>

//--- Triangle ---

Triangle::Triangle(int primID, int vtx0Idx, int vtx1Idx, int vtx2Idx,
                   const TriangleMesh &mesh)
    : primID_(primID),
      v0_(mesh.transform.toWorld(mesh.meshData->vertexBuffer[vtx0Idx])),
      v1_(mesh.transform.toWorld(mesh.meshData->vertexBuffer[vtx1Idx])),
      v2_(mesh.transform.toWorld(mesh.meshData->vertexBuffer[vtx2Idx])) {
    boundingBox.Expand(v0_);
    boundingBox.Expand(v1_);
    boundingBox.Expand(v2_);
    geometryID_ = mesh.geometryID_;
}

bool Triangle::rayIntersectShape(Ray &ray, int &primID, float &u,
                                 float &v) const {
    const auto &origin = ray.origin;
    const auto &direction = ray.direction;

    auto edge1 = v1_ - v0_;
    auto edge2 = v2_ - v0_;

    auto norm = cross(edge1, edge2);
    auto det = -dot(direction, norm);
    if (std::abs(det) < EPSILON) // parallel
        return false;

    auto inv_det = 1.f / det;
    auto to = origin - v0_;
    auto tmp_u = dot(to, cross(direction, edge2)) * inv_det;
    auto tmp_v = dot(direction, cross(to, edge1)) * inv_det;
    if (tmp_u < .0f || tmp_v < .0f || tmp_u + tmp_v > 1.f)
        return false;

    auto t = dot(edge2, cross(to, edge1)) * inv_det;
    if (t < ray.tNear || t > ray.tFar)
        return false;

    ray.tFar = t;
    primID = primID_;
    u = tmp_u;
    v = tmp_v;

    return true;
}

void Triangle::fillIntersection(float distance, int primID, float u, float v,
                                Intersection &intersection) const {
    Todo();
    return;
}

//--- TriangleMesh ---
TriangleMesh::TriangleMesh(const Json &json) : Shape(json) {
    const auto &filepath = fetchRequired<std::string>(json, "file");
    meshData = MeshData::loadFromFile(filepath);
}

RTCGeometry TriangleMesh::getEmbreeGeometry(RTCDevice device) const {
    RTCGeometry geometry = rtcNewGeometry(device, RTC_GEOMETRY_TYPE_TRIANGLE);

    float *vertexBuffer = (float *)rtcSetNewGeometryBuffer(
        geometry, RTC_BUFFER_TYPE_VERTEX, 0, RTC_FORMAT_FLOAT3,
        3 * sizeof(float), meshData->vertexCount);
    for (int i = 0; i < meshData->vertexCount; ++i) {
        Point3f vertex = transform.toWorld(meshData->vertexBuffer[i]);
        vertexBuffer[3 * i] = vertex[0];
        vertexBuffer[3 * i + 1] = vertex[1];
        vertexBuffer[3 * i + 2] = vertex[2];
    }

    unsigned *indexBuffer = (unsigned *)rtcSetNewGeometryBuffer(
        geometry, RTC_BUFFER_TYPE_INDEX, 0, RTC_FORMAT_UINT3,
        3 * sizeof(unsigned), meshData->faceCount);
    for (int i = 0; i < meshData->faceCount; ++i) {
        indexBuffer[i * 3] = meshData->faceBuffer[i][0].vertexIndex;
        indexBuffer[i * 3 + 1] = meshData->faceBuffer[i][1].vertexIndex;
        indexBuffer[i * 3 + 2] = meshData->faceBuffer[i][2].vertexIndex;
    }
    rtcCommitGeometry(geometry);
    return geometry;
}

bool TriangleMesh::rayIntersectShape(Ray &ray, int &primID, float &u,
                                     float &v) const {
    //* 当使用embree加速时，该方法不会被调用
    int geomID = -1;
    return acceleration->rayIntersect(ray, geomID, primID, u, v);
}

void TriangleMesh::fillIntersection(float distance, int primID, float u,
                                    float v, Intersection &intersection) const {
    intersection.distance = distance;
    intersection.shape = this;

    auto faceInfo = meshData->faceBuffer[primID];
    auto w = 1.f - u - v;

    // 交点
    auto pw =
             transform.toWorld(meshData->vertexBuffer[faceInfo[0].vertexIndex]),
         pu =
             transform.toWorld(meshData->vertexBuffer[faceInfo[1].vertexIndex]),
         pv =
             transform.toWorld(meshData->vertexBuffer[faceInfo[2].vertexIndex]);
    intersection.position = Point3f{w * pw[0] + u * pu[0] + v * pv[0],
                                    w * pw[1] + u * pu[1] + v * pv[1],
                                    w * pw[2] + u * pu[2] + v * pv[2]};
    // 法线
    if (meshData->normalBuffer.empty()) {
        intersection.normal = normalize(cross(pu - pw, pv - pw));
    } else {
        Vector3f nw = transform.toWorld(
                     meshData->normalBuffer[faceInfo[0].normalIndex]),
                 nu = transform.toWorld(
                     meshData->normalBuffer[faceInfo[1].normalIndex]),
                 nv = transform.toWorld(
                     meshData->normalBuffer[faceInfo[2].normalIndex]);
        intersection.normal = normalize(w * nw + u * nu + v * nv);
    }

    // 纹理坐标
    if (meshData->texcodBuffer.empty()) {
        intersection.texCoord = Vector2f::zero();
    } else {
        Vector2f tw = meshData->texcodBuffer[faceInfo[0].texcodIndex],
                 tu = meshData->texcodBuffer[faceInfo[1].texcodIndex],
                 tv = meshData->texcodBuffer[faceInfo[2].texcodIndex];
        intersection.texCoord = w * tw + u * tu + v * tv;
    }

    // 切线空间
    Vector3f tangent{1.f, 0.f, .0f};
    Vector3f bitangent;
    if (std::abs(dot(tangent, intersection.normal)) > .9f) {
        tangent = Vector3f(.0f, 1.f, .0f);
    }
    bitangent = normalize(cross(tangent, intersection.normal));
    tangent = normalize(cross(intersection.normal, bitangent));
    intersection.tangent = tangent;
    intersection.bitangent = bitangent;
}

void TriangleMesh::initInternalAcceleration() {
    acceleration = Acceleration::createAcceleration();
    int primCount = meshData->faceCount;
    for (int primID = 0; primID < primCount; ++primID) {
        int vtx0Idx = meshData->faceBuffer[primID][0].vertexIndex,
            vtx1Idx = meshData->faceBuffer[primID][1].vertexIndex,
            vtx2Idx = meshData->faceBuffer[primID][2].vertexIndex;
        std::shared_ptr<Triangle> triangle = std::make_shared<Triangle>(
            primID, vtx0Idx, vtx1Idx, vtx2Idx, *this);
        acceleration->attachShape(triangle);
    }
    acceleration->build();
    // TriangleMesh的包围盒就是其内部加速结构的包围盒
    boundingBox = acceleration->boundingBox;
}
REGISTER_CLASS(TriangleMesh, "triangle")
