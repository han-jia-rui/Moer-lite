#include "VolumeIntegrator.h"
#include "FunctionLayer/Volume/Volume.h"
#include <FunctionLayer/Material/Material.h>

inline Spectrum lightEmission(const Intersection &intersection,
                              const Ray &ray) {
    Spectrum radiance(0.0f);
    if (auto light = intersection.shape->light; light) {
        radiance += light->evaluateEmission(intersection, -ray.direction);
    }
    return radiance;
}

Spectrum VolumeIntegrator::li(Ray &ray, const Scene &scene,
                              std::shared_ptr<Sampler> sampler) const {
    Spectrum radiance(.0f), weight(1.0f);
    std::vector<std::shared_ptr<Material>> materials;

    while (true) {
        // Russian roulette termination
        auto rrProb = std::max(.05f, weight.maxComponent());
        if (sampler->next1D() > rrProb) {
            break; // Terminate the ray
        }
        weight /= rrProb; // Continue with reduced weight

        // Check if the ray intersects any shape
        auto itsOpt = scene.rayIntersect(ray);

        // if no intersection, accumulate light from infinite lights
        if (!itsOpt.has_value()) {
            // assert(materials.empty()); // the ray should not in any material
            for (auto light : scene.infiniteLights) {
                radiance += weight * light->evaluateEmission(ray);
            }
            break;
        }

        Intersection intersection = itsOpt.value();
        computeRayDifferentials(intersection, ray);

        // compute the direct illumination
        radiance += weight * lightEmission(intersection, ray);

        auto material = intersection.shape->material;
        auto bsdf = material->createBSDF(intersection);

        // compute the BSDF sample
        auto wo = -ray.direction;
        auto bsdfSampleResult = bsdf->sample(wo, sampler->next2D());
        weight *= bsdfSampleResult.weight / bsdfSampleResult.pdf;

        if (material->isVolume()) {
            float wo_dot_normal = dot(wo, intersection.normal);
            float wi_dot_normal = dot(bsdfSampleResult.wi, intersection.normal);
            bool transmit = wo_dot_normal * wi_dot_normal <
                            0; // check if we transmit through the surface
            if (transmit) {
                bool entered =
                    wo_dot_normal > 0; // check if we entered the volume
                if (entered) {
                    materials.push_back(material);
                } else {
                    materials.erase(std::remove(materials.begin(),
                                                materials.end(), material),
                                    materials.end());
                }
            }
        }

        if (!materials.empty()) {
            auto currentVolume = materials.back()->createVolume(intersection);
            auto volumeResult = currentVolume->integrate(scene, ray);
            if (volumeResult.has_value()) {
                auto ctx = volumeResult.value();
                radiance += weight * ctx.L;
                weight *= ctx.transmittance;
                ray = ctx.wo;
                continue;
            }
        }

        ray = Ray(intersection.position, bsdfSampleResult.wi);
    };

    return radiance;
}

REGISTER_CLASS(VolumeIntegrator, "volume")
