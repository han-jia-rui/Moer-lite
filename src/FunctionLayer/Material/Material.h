#pragma once
#include <CoreLayer/ColorSpace/Spectrum.h>
#include <CoreLayer/Math/Math.h>
#include <FunctionLayer/Shape/Intersection.h>
#include <FunctionLayer/Texture/NormalTexture.h>
#include <ResourceLayer/Factory.h>
#include <ResourceLayer/JsonUtil.h>

enum class BSDFType { Diffuse, Specular };

struct BSDFSampleResult {
    Spectrum weight;
    Vector3f wi;
    float pdf;
    BSDFType type;
};

class BSDF {
  public:
    BSDF(const Vector3f &_normal, const Vector3f &_tangent,
         const Vector3f &_bitangent)
        : normal(_normal), tangent(_tangent), bitangent(_bitangent) {}

    virtual ~BSDF() = default;

    virtual Spectrum f(const Vector3f &wo, const Vector3f &wi) const = 0;
    virtual BSDFSampleResult sample(const Vector3f &wo,
                                    const Vector2f &sample) const = 0;

    Vector3f normal, tangent, bitangent; // 构成局部坐标系
  protected:
    Vector3f toLocal(Vector3f world) const {
        return Vector3f{dot(tangent, world), dot(normal, world),
                        dot(bitangent, world)};
    }
    Vector3f toWorld(Vector3f local) const {
        return local[0] * tangent + local[1] * normal + local[2] * bitangent;
    }
};

class Material {
  public:
    Material() = default;

    virtual ~Material() = default;

    explicit Material(const Json &json) {
        if (json.contains("normalmap"))
            normalMap = std::make_shared<NormalTexture>(json["normalmap"]);
    }

    virtual std::shared_ptr<BSDF>
    computeBSDF(const Intersection &intersection) const = 0;

    void computeShadingGeometry(const Intersection &intersection,
                                Vector3f &normal, Vector3f &tangent,
                                Vector3f &bitangent) const;

  protected:
    std::shared_ptr<NormalTexture> normalMap;
};

inline Vector3f squareToUniformHemisphere(Vector2f sample) {
    float y = 1 - 2 * sample[0];
    float r = fm::sqrt(std::max(.0f, 1.f - y * y));
    float phi = 2 * PI * sample[1];
    Vector3f dir{r * fm::sin(phi), std::abs(y), r * fm::cos(phi)};
    return normalize(dir);
}

inline float squareToUniformHemispherePdf(Vector3f v) {
    return v[1] >= .0f ? INV_PI * .5f : .0f;
}

inline Vector3f squareToCosineHemisphere(Vector2f sample) {
    float phi = 2 * M_PI * sample[0], theta = std::acos(sample[1]);
    return Vector3f{std::sin(theta) * std::sin(phi), std::cos(theta),
                    std::sin(theta) * std::cos(phi)};
}

inline float squareToCosineHemispherePdf(Vector3f v) {
    return (v[1] > .0f) ? v[1] * INV_PI : .0f;
}
