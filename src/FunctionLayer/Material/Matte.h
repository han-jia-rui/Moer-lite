#pragma once
#include "Material.h"

class MatteMaterial : public Material {
  public:
    MatteMaterial();

    explicit MatteMaterial(const Json &json);

    std::shared_ptr<BSDF>
    createBSDF(const Intersection &intersection) const override;

  private:
    std::shared_ptr<Texture<Spectrum>> albedo;
};
