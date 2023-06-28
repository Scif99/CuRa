#ifndef VERTEX_H
#define VERTEX_H

#include  "vec.h"



enum class VertexAttribute {
    Position,
    Normal,
    TexCoord
};

//Processed by the vertex shader.
//Contains per-vertex attributes.
struct Vertex {
    Vec3f Position;
    Vec3f Normal;
    Vec2f TexCoord; 
};

//Produced by the vertex shader.
//Is returned form the vertex shader.
struct ShadedVertex {
    Vec4f clip_coords;
    Vec3f world_norm;
    Vec2f tex_coords;
};

//Processed by the fragment shader.
//Contains all the data needed in the fragment shader.
struct Fragment{
    Vec3f window_coords;
    Vec3f normal;
    Vec2f tex_coords;
};

//Produced by the fragment shader.
// Contains the data needed to write to an image buffer.
struct ShadedFragment {
    Vec3f pos; //Contains window coordinates as well as the fragment depth.
    Color3f FragColor; //rgb color.
};

#endif  