#pragma once
#include "Volume.h"
#include <CoreLayer/ColorSpace/Spectrum.h>
#include <CoreLayer/Math/Math.h>
#include <FunctionLayer/Sampler/IndependentSampler.h>
#include <FunctionLayer/Scene/Scene.h>
#include <FunctionLayer/Shape/Intersection.h>

inline float convertPDF(const LightSampleResult &result,
                        const Intersection &intersection) {
    float pdf = result.pdf;
    float distance = result.distance;
    switch (result.type) {
    case LightType::SpotLight:
        pdf *= distance * distance;
        break;
    case LightType::AreaLight:
        pdf *= distance * distance;
        pdf /= std::abs(dot(result.normal, result.direction));
        break;
    //* 环境光的pdf转换在采样时已经完成
    case LightType::EnvironmentLight:
        break;
    }
    return pdf;
}

inline float sampler() {
    static std::random_device rd;
    static auto gen = std::mt19937(rd());
    static auto dist = std::uniform_real_distribution<float>(.0f, 1.f);
    return dist(gen);
}

inline float isotropicPDF() { return 0.25f * INV_PI; }

inline void isotropicGenerate(Vector3f &direction, Spectrum &L, float &pdf) {
    auto xi = sampler();
    direction.z() = 1.0f - 2.0f * xi; // Cosine-weighted hemisphere sampling
    auto sinTheta = 1.0f - direction.z() * direction.z();
    if (sinTheta < 0.0f) {
        direction.x() = 0.0f; // Handle numerical issues
        direction.y() = 0.0f;
    } else {
        sinTheta = std::sqrt(sinTheta);
        xi = sampler();
        float phi = 2.0f * PI * xi; // Uniform azimuthal angle
        direction.x() = sinTheta * std::cos(phi);
        direction.y() = sinTheta * std::sin(phi);
    }
    pdf = 0.25f * INV_PI; // PDF for isotropic sampling
    L = Spectrum(pdf);    // Assuming uniform radiance for simplicity
}

class SingleScatterHomogeneousVolume : public Volume {
  public:
    SingleScatterHomogeneousVolume(const Spectrum &sigmaT,
                                   const Spectrum &albedo, Intersection its)
        : Volume(its), m_sigmaT(sigmaT), m_albedo(albedo),
          m_sigmaS(sigmaT * albedo), m_sigmaA(sigmaT - m_sigmaS) {}

    virtual ~SingleScatterHomogeneousVolume() = default;

    virtual std::optional<VolumeContext> integrate(const Scene &scene,
                                                   const Ray &wi) override {
        // Create ray from current position in the direction of wi
        Ray rayToTest(m_its.position, wi.direction);
        auto itsOpt = scene.rayIntersect(rayToTest);

        if (!itsOpt.has_value()) {
            return std::nullopt;
        }

        auto hitResult = itsOpt.value();

        auto t = -std::log(1.0f - sampler()) / m_sigmaT.r();

        if (t > hitResult.distance) {
            VolumeContext volume_ctx{
                .wo = Ray(hitResult.position, wi.direction),
                .p = hitResult.position,
                .L = Spectrum(0.0f),      // No radiance contribution
                .weight = Spectrum(1.0f), // Full weight
                .transmittance =
                    computeTransmittance(hitResult.position, m_its.position),
            };

            return std::make_optional(volume_ctx);
        }

        Point3f Pscatter = m_its.position + t * wi.direction;

        auto Tr = computeTransmittance(m_its.position, Pscatter);
        m_its.position = Pscatter;

        VolumeContext volume_ctx{
            .wo = Ray(Pscatter, wi.direction),
            .p = Pscatter,
            .L = Spectrum(0.8f) / m_albedo, // No radiance contribution
            .weight = Spectrum(1.0f),       // Full weight
            .transmittance = Spectrum(0.0f),
        };

        return std::make_optional(volume_ctx);
    }

    virtual Spectrum computeTransmittance(const Point3f &p0,
                                          const Point3f &p1) override {
        Vector3f diff = p0 - p1;
        float distance = diff.length();

        // Apply Beer's Law: T = exp(-sigma_a * distance)
        return Spectrum(std::exp(-m_sigmaT.r() * distance),
                        std::exp(-m_sigmaT.g() * distance),
                        std::exp(-m_sigmaT.b() * distance));
    }

  protected:
    const Spectrum m_sigmaT, m_albedo, m_sigmaS, m_sigmaA;
};
