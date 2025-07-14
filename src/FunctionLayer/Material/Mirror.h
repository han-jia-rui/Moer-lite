#pragma once
#include "Material.h"

class MirrorMaterial : public Material {
  public:
    MirrorMaterial() = delete;

    MirrorMaterial(const Json &json) : Material(json) {};

    virtual std::shared_ptr<BSDF>
    createBSDF(const Intersection &intersection) const override;
};
