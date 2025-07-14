#pragma once
#include "Material.h"
#include <CoreLayer/ColorSpace/Spectrum.h>
#include <FunctionLayer/BxDF/Trivial.h>
#include <FunctionLayer/Volume/SingleScatterHomogeneous.h>
#include <ResourceLayer/Factory.h>
#include <ResourceLayer/JsonUtil.h>

class HomogeneousMaterial : public Material {
  public:
    HomogeneousMaterial() = default;
    explicit HomogeneousMaterial(const Json &json);

    virtual std::shared_ptr<BSDF>
    createBSDF(const Intersection &intersection) const override {
        Vector3f normal, tangent, bitangent;
        computeShadingGeometry(intersection, normal, tangent, bitangent);
        return std::make_shared<TrivialBSDF>(normal, tangent, bitangent);
    }

    virtual bool isVolume() const override { return true; }

    virtual std::shared_ptr<Volume>
    createVolume(const Intersection &intersection) const override {
        return std::make_shared<SingleScatterHomogeneousVolume>(
            m_sigmaT, m_albedo, intersection);
    }

  private:
    Spectrum m_sigmaT, m_albedo;
};
