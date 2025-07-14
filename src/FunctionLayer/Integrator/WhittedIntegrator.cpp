#include "WhittedIntegrator.h"
#include <FunctionLayer/Material/Material.h>

Spectrum WhittedIntegrator::li(Ray &ray, const Scene &scene,
                               std::shared_ptr<Sampler> sampler) const {
    Spectrum radiance(.0f), weight(1.0f);
    while (true) {
        auto itsOpt = scene.rayIntersect(ray);

        // escape the scene
        if (!itsOpt.has_value()) {
            for (auto light : scene.infiniteLights) {
                radiance += weight * light->evaluateEmission(ray);
            }
            break;
        }

        Intersection intersection = itsOpt.value();
        if (auto light = intersection.shape->light; light) {
            radiance +=
                weight * light->evaluateEmission(intersection, -ray.direction);
        }
        computeRayDifferentials(intersection, ray);
        auto bsdf = intersection.shape->material->createBSDF(intersection);

        auto bsdfSampleResult = bsdf->sample(-ray.direction, sampler->next2D());

        if (bsdfSampleResult.type == BSDFType::Specular) {
            // If the surface is specular, spawn the ray
            ray = Ray(intersection.position, bsdfSampleResult.wi);
            weight *= bsdfSampleResult.weight;
            continue;
        } else {
            // If the surface is not specular, sample the light
            // First, sample infinite light
            for (auto light : scene.infiniteLights) {
                auto lightSampleResult =
                    light->sample(intersection, sampler->next2D());
                Ray shadowRay(intersection.position,
                              lightSampleResult.direction, 1e-4f, FLT_MAX);
                // Successfully connect the light source
                if (auto occlude = scene.rayIntersect(shadowRay);
                    !occlude.has_value()) {
                    Spectrum f = bsdf->f(-ray.direction, shadowRay.direction);
                    float pdf = convertPDF(lightSampleResult, intersection);
                    radiance += weight * lightSampleResult.energy * f / pdf;
                }
            }

            float pdfLight = .0f;
            // Second, sample the light in scene
            auto light = scene.sampleLight(sampler->next1D(), &pdfLight);
            if (light && pdfLight != .0f) {
                auto lightSampleResult =
                    light->sample(intersection, sampler->next2D());
                Ray shadowRay(intersection.position,
                              lightSampleResult.direction, 1e-4f,
                              lightSampleResult.distance);
                // Successfully connect the light source
                if (auto occlude = scene.rayIntersect(shadowRay);
                    !occlude.has_value()) {
                    Spectrum f = bsdf->f(-ray.direction, shadowRay.direction);
                    lightSampleResult.pdf *= pdfLight;
                    float pdf = convertPDF(lightSampleResult, intersection);
                    radiance += weight * lightSampleResult.energy * f / pdf;
                }
            }
            break;
        }
    };

    return radiance;
}

REGISTER_CLASS(WhittedIntegrator, "whitted")
