#ifndef VERTEX_H
#define VERTEX_H

#include  "vec.h"

//Contains per-vertex attributes before vertex shader has been run
struct VertexAttributes {
    Vec3f Position;
    Vec3f Normal;
    Vec2f TexCoord; 
};

//Contains the vertex data after it has been run throught the vertex shader
struct ProcessedVertex {
    Vec4f clip_coords;
    Vec2f tex_coords;
    Color3f diffuse;
};

//Contains all the data needed in the fragment shader
struct Fragment {
    Vec3f window_coords;
    Vec2f tex_coords;
    Color3f diffuse;
};

#endif  