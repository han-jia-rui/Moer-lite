#pragma once
#include "CoreLayer/Math/Geometry.h"
#include <CoreLayer/ColorSpace/Spectrum.h>
#include <CoreLayer/Math/Math.h>
#include <FunctionLayer/Scene/Scene.h>
#include <FunctionLayer/Shape/Intersection.h>
#include <optional>

struct VolumeContext {
    Ray wo;
    Point3f p;       // the hit point
    Spectrum L;      // the radiance along the Ray
    Spectrum weight; // the weight of the Ray
    Spectrum transmittance;
};

class Volume {
  public:
    explicit Volume(Intersection its) : m_its(its) {}

    virtual ~Volume() = default;

    virtual std::optional<VolumeContext> integrate(const Scene &scene,
                                                   const Ray &wi) = 0;

    virtual Spectrum computeTransmittance(const Point3f &p0,
                                          const Point3f &p1) = 0;

  protected:
    Intersection m_its;
};

class EmptyVolume : public Volume {
  public:
    explicit EmptyVolume(Intersection its) : Volume(its) {}

    ~EmptyVolume() = default;

    virtual std::optional<VolumeContext> integrate(const Scene &scene,
                                                   const Ray &wi) override {
        Ray rayToTest(m_its.position, wi.direction);
        auto itsOpt = scene.rayIntersect(rayToTest);
        if (!itsOpt.has_value()) {
            return std::nullopt;
        }
        auto hitResult = itsOpt.value();

        // Set volume context results
        VolumeContext volume_ctx{
            .wo = Ray(hitResult.position, wi.direction),
            .p = hitResult.position,  // Hit point
            .L = Spectrum(0.0f),      // No radiance in empty volume
            .weight = Spectrum(1.0f), // Full weight
            .transmittance = computeTransmittance(
                hitResult.position, m_its.position),
        };

        return std::make_optional(volume_ctx);
    }

    virtual Spectrum computeTransmittance(const Point3f & /*p0*/,
                                          const Point3f & /*p1*/) override {
        return Spectrum(1.0f); // No transmittance loss in empty volume
    }
};
