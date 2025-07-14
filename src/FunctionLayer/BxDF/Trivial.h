#pragma once
#include "BSDF.h"

// Trivial BSDF for volume materials that don't scatter at surfaces
class TrivialBSDF : public BSDF {
  public:
    TrivialBSDF(const Vector3f &normal, const Vector3f &tangent,
                const Vector3f &bitangent)
        : BSDF(normal, tangent, bitangent) {}

    virtual Spectrum f(const Vector3f & /*wo*/,
                       const Vector3f & /*wi*/) const override {
        return Spectrum(0.0f); // No surface scattering
    }

    virtual BSDFSampleResult
    sample(const Vector3f &wo, const Vector2f & /*sample*/) const override {
        BSDFSampleResult result;
        result.weight = Spectrum(1.0f);
        result.wi = -wo; // Reflective direction);
        result.pdf = 1.0f;
        result.type = BSDFType::Specular;
        return result;
    }
};
