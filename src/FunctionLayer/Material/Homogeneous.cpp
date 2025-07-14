#include "Homogeneous.h"

HomogeneousMaterial::HomogeneousMaterial(const Json &json) : Material(json) {
    if (json.contains("sigmaT")) {
        if (json["sigmaT"].is_array()) {
            m_sigmaT = fetchRequired<Spectrum>(json, "sigmaT");
        } else {
            float delta = fetchRequired<float>(json, "sigmaT");
            m_sigmaT = Spectrum(delta);
        }
    } else {
        m_sigmaT = Spectrum(0.1f); // Default extinction
    }
    if (json.contains("albedo")) {
        if (json["albedo"].is_array()) {
            m_albedo = fetchRequired<Spectrum>(json, "albedo");
        } else {
            float delta = fetchRequired<float>(json, "albedo");
            m_albedo = Spectrum(delta);
        }
    } else {
        m_albedo = Spectrum(0.1f);
    }
}

REGISTER_CLASS(HomogeneousMaterial, "homogeneous");
