#ifndef LIGHT_H
#define LIGHT_H

#include "vec.h"

struct DistantLight {
    Norm3f Direction;
    Color3f Ambient;
    Color3f Diffuse;
    Color3f Specular;
};

struct PointLight {
    Vec3f Position;
    Color3f Diffuse;
    Color3f Specular;
};

#endif  