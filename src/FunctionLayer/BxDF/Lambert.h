#pragma once

#include "BSDF.h"

class LambertBSDF : public BSDF {
  public:
    LambertBSDF(const Vector3f &_normal, const Vector3f &_tangent,
                const Vector3f &_bitangent, Spectrum _albedo)
        : BSDF(_normal, _tangent, _bitangent), albedo(_albedo) {}

    virtual ~LambertBSDF() = default;

    virtual Spectrum f(const Vector3f & /*wo*/,
                       const Vector3f &wi) const override {
        Vector3f wiLocal = toLocal(wi);
        return albedo * (INV_PI * std::max(wiLocal[1], 0.0f));
    }

    virtual BSDFSampleResult sample(const Vector3f & /*wo*/,
                                    const Vector2f &sample) const override {
        Vector3f wi = squareToCosineHemisphere(sample); // 入射角
        float pdf = squareToCosineHemispherePdf(wi);
        return {albedo, toWorld(wi), pdf, BSDFType::Diffuse};
    }

  private:
    Spectrum albedo;
};
