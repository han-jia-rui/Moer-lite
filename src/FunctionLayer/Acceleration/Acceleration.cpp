#include "Acceleration.h"
#include "BVH.h"
#include "EmbreeBVH.h"
#include "Linear.h"
#include "Octree.h"
// Default acceleration type is embree
AccelerationType Acceleration::type = AccelerationType::Embree;

std::map<std::string, AccelerationType> accelerationTypeMap = {
    {"embree", AccelerationType::Embree},
    {"linear", AccelerationType::Linear},
    {"octree", AccelerationType::Octree},
    {"bvh", AccelerationType::BVH},
};

std::map<AccelerationType, std::function<std::shared_ptr<Acceleration>()>>
    accelerationBuildMap = {
        {AccelerationType::Embree, std::make_shared<EmbreeBVH>},
        {AccelerationType::Linear, std::make_shared<LinearAcceleration>},
        {AccelerationType::Octree, std::make_shared<Octree>},
        {AccelerationType::BVH, std::make_shared<BVH>},
};

void Acceleration::setAccelerationType(const std::string &type_) {
    if (accelerationTypeMap.count(type_) == 0) {
        std::cerr << "Unknown acceleration type " << type_ << "!\n";
        exit(1);
    }
    std::cout << "Using acceleration type " << type_ << "\n";
    Acceleration::type = accelerationTypeMap[type_];
}

std::shared_ptr<Acceleration> Acceleration::createAcceleration() {
    return accelerationBuildMap[Acceleration::type]();
}
