#ifndef LIGHT_H
#define LIGHT_H

#include "vec.h"

struct DistantLight {
    Norm3f Direction;
    Color3f Color;
};

#endif  