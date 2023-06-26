#ifndef TRIANGLE_H
#define TRIANGLE_H

#include <array>
#include "vertex.h"

//Note that the vertices need to be specified in CCW order
struct Triangle {
    std::array<ProcessedVertex, 3> vertices;
};

#endif