#pragma once
#include "Volume.h"
#include <CoreLayer/ColorSpace/Spectrum.h>
#include <CoreLayer/Math/Math.h>
#include <FunctionLayer/Scene/Scene.h>
#include <FunctionLayer/Shape/Intersection.h>

class BeersLawVolume : public Volume {
public:
    BeersLawVolume(const Spectrum &absorption, Intersection its) 
        : Volume(its), m_absorption(absorption) {}

    virtual ~BeersLawVolume() = default;

    virtual std::optional<VolumeContext> integrate(const Scene &scene,
                                                   const Ray &wi) override {
        // Create ray from current position in the direction of wi
        Ray rayToTest(m_its.position, wi.direction);
        auto itsOpt = scene.rayIntersect(rayToTest);
        
        if (!itsOpt.has_value()) {
            return std::nullopt;
        }
        
        auto hitResult = itsOpt.value();

        // Set volume context results
        VolumeContext volume_ctx{
            .wo = Ray(hitResult.position, wi.direction),
            .p = hitResult.position,
            .L = Spectrum(0.0f), // No radiance contribution
            .weight = Spectrum(1.0f), // Full weight
            .transmittance = computeTransmittance(hitResult.position, m_its.position),
        };

        return std::make_optional(volume_ctx);
    }

    virtual Spectrum computeTransmittance(const Point3f &p0,
                                         const Point3f &p1) override {
        Vector3f diff = p0 - p1;
        float distance = diff.length();
        
        // Apply Beer's Law: T = exp(-sigma_a * distance)
        // For RGB channels separately
        return Spectrum(std::exp(-m_absorption.r() * distance),
                       std::exp(-m_absorption.g() * distance),
                       std::exp(-m_absorption.b() * distance));
    }

protected:
    const Spectrum m_absorption;
};
