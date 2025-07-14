#pragma once
#include "Material.h"
#include <FunctionLayer/Volume/BeersLaw.h>
#include <FunctionLayer/BxDF/Trivial.h>
#include <CoreLayer/ColorSpace/Spectrum.h>
#include <ResourceLayer/Factory.h>
#include <ResourceLayer/JsonUtil.h>

class BeersLawMaterial : public Material {
public:
    BeersLawMaterial() = default;
    explicit BeersLawMaterial(const Json &json);

    virtual std::shared_ptr<BSDF> createBSDF(const Intersection &intersection) const override {
        Vector3f normal, tangent, bitangent;
        computeShadingGeometry(intersection, normal, tangent, bitangent);
        return std::make_shared<TrivialBSDF>(normal, tangent, bitangent);
    }

    virtual bool isVolume() const override {
        return true;
    }

    virtual std::shared_ptr<Volume> createVolume(const Intersection &intersection) const override {
        return std::make_shared<BeersLawVolume>(m_absorption, intersection);
    }

private:
    Spectrum m_absorption = Spectrum(0.1f); // Default extinction
};
