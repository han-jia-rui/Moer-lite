#pragma once
#include <CoreLayer/ColorSpace/Spectrum.h>
#include <CoreLayer/Math/Math.h>
#include <FunctionLayer/BxDF/BSDF.h>
#include <FunctionLayer/Shape/Intersection.h>
#include <FunctionLayer/Texture/NormalTexture.h>
#include <FunctionLayer/Volume/Volume.h>
#include <ResourceLayer/Factory.h>
#include <ResourceLayer/JsonUtil.h>

class Material {
  public:
    Material() = default;

    virtual ~Material() = default;

    explicit Material(const Json &json) {
        if (json.contains("normalmap"))
            normalMap = std::make_shared<NormalTexture>(json["normalmap"]);
    }

    virtual std::shared_ptr<BSDF>
    createBSDF(const Intersection &intersection) const = 0;

    virtual bool isVolume() const { return false; }

    virtual std::shared_ptr<Volume> createVolume(const Intersection &) const {
        return nullptr;
    };

  protected:
    void computeShadingGeometry(const Intersection &intersection,
                                Vector3f &normal, Vector3f &tangent,
                                Vector3f &bitangent) const;

    std::shared_ptr<NormalTexture> normalMap;
};
