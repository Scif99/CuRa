#ifndef DEPTH_SHADER_H
#define DEPTH_SHADER_H

#include "shader.h"

class DepthShader : public Shader {
public:
    [[nodiscard]] ShadedVertex PerVertex(const Vertex& vertex) override;
    [[nodiscard]] ShadedFragment PerFragment(const Fragment& frag) override;
};

#endif