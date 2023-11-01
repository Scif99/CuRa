#pragma once

#include <cura/math.h>




//A vertex that has been clipped & mapped to viewport.
//Is passed to the rasterizer.
struct ClippedVertex {
    Vec3f pixel_coords;
    Vec2f tex_coords;
};

