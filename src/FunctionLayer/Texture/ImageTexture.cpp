#include "ImageTexture.h"
#include <ResourceLayer/Factory.h>

ImageTexture::ImageTexture(const Json &json) : Texture<Spectrum>() {
    std::string filepath = fetchRequired<std::string>(json, "file");
    filepath = FileUtil::getFullPath(filepath);
    auto image = loadImage(filepath.c_str());
    size = image->size;
    mipmap = std::make_shared<MipMap>(image);
}

Spectrum ImageTexture::evaluate(const Intersection &intersection) const {
    TextureCoord texCoord = mapping->map(intersection);
    return evaluate(texCoord);
}

Spectrum ImageTexture::evaluate(const TextureCoord &texCoord) const {
    return Spectrum{
        mipmap->lookUp(texCoord.coord, texCoord.duv_dx, texCoord.duv_dy)};
}

REGISTER_CLASS(ImageTexture, "imageTex")
