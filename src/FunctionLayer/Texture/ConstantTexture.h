#pragma once

#include "Texture.h"
#include <CoreLayer/ColorSpace/Spectrum.h>
template <typename TReturn>
class ConstantTexture : public Texture<TReturn> {
  public:
    explicit ConstantTexture(const TReturn &_data) : data(_data) {}

    virtual TReturn evaluate(const Intersection &) const override {
        return data;
    }

    virtual TReturn evaluate(const TextureCoord &) const override {
        return data;
    }

  private:
    TReturn data;
};
