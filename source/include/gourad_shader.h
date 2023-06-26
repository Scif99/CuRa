#ifndef GOURAD_H
#define GOURAD_H

#include "buffer.h"
#include "shader.h"
#include "vertex.h"

class GouradShader : public Shader{
public:

    [[nodiscard]] ProcessedVertex PerVertex(const VertexAttributes& v_attributes) override;

    /*Processes a rasterised fragment.
    The input should contain AT LEAST:
        - Window coordinates
        - Interpolated per-vertex attributes

    The output is a 3d vector (2d pixel coords, depth) and a color
    */
    [[nodiscard]] FragmentData PerFragment(const Fragment& frag) override;


};

#endif