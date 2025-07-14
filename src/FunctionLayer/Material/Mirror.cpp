#include "Mirror.h"
#include <FunctionLayer/BxDF/Specular.h>

std::shared_ptr<BSDF>
MirrorMaterial::createBSDF(const Intersection &intersection) const {
    Vector3f normal, tangent, bitangent;
    computeShadingGeometry(intersection, normal, tangent, bitangent);
    return std::make_shared<SpecularBSDF>(normal, tangent, bitangent);
}

REGISTER_CLASS(MirrorMaterial, "mirror")
