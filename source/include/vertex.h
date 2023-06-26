#ifndef VERTEX_H
#define VERTEX_H

#include  "vec.h"

//Contains per-vertex attributes.
//Used as input for the vertex shader
struct VertexAttributes {
    Vec3f Position;
    Vec3f Normal;
    Vec2f TexCoord; 
};

//Contains any data needed in the pipeline between the vertex shader and the fracment shader 
//Is returned form the vertex shader
struct ProcessedVertex {
    Vec4f clip_coords;
    Vec3f world_norm;
    Vec2f tex_coords;
};

//Contains all the data needed in the fragment shader
//Used as input for the fragment shader
struct Fragment {
    Vec3f window_coords;
    Vec3f normal;
    Vec2f tex_coords;
};

// Contains the data needed to write to an image buffer
//Is returnged from the fragment shader
struct FragmentData {
    Vec3f pos; //Contains window coordinates as well as the fragment depth
    Color3f FragColor; //rgb color
};

#endif  