#pragma once
#include "Shape.h"

class Cube : public Shape {
  public:
    Cube() = delete;

    explicit Cube(const Json &json);

    virtual bool rayIntersectShape(Ray &ray, int &primID, float &u,
                                   float &v) const override;

    virtual void fillIntersection(float distance, int primID, float u, float v,
                                  Intersection &intersection) const override;

    virtual void uniformSampleOnSurface(Vector2f sample, Intersection &result,
                                        float *pdf) const override {
        // TODO: finish this
    }

  protected:
    Point3f boxMin, boxMax;
};
