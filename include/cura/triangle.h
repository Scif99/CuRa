#pragma once

#include <array>
#include "vertex.h"

//Note that the vertices need to be specified in CCW order
struct Triangle {
    std::array<ShadedVertex, 3> vertices;
};

