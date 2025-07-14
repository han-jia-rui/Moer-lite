#include "BeersLaw.h"

BeersLawMaterial::BeersLawMaterial(const Json &json) : Material(json) {
    if (json.contains("absorption")) {
        if (json["absorption"].is_array()) {
            m_absorption = fetchRequired<Spectrum>(json, "absorption");
        } else {
            float delta = fetchRequired<float>(json, "absorption");
            m_absorption = Spectrum(delta);
        }
    } else {
        m_absorption = Spectrum(0.1f); // Default extinction
    }
}

REGISTER_CLASS(BeersLawMaterial, "beerslaw")
