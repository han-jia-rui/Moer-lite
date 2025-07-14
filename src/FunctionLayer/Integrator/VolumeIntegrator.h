#pragma once

#include "Integrator.h"

class VolumeIntegrator : public Integrator {
  public:
    VolumeIntegrator() = default;

    explicit VolumeIntegrator(const Json &json) : Integrator(json) {}

    virtual ~VolumeIntegrator() = default;

    virtual Spectrum li(Ray &ray, const Scene &scene,
                        std::shared_ptr<Sampler> sampler) const override;
};
