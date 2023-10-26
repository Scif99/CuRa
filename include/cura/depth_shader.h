#pragma once

#include <cura/shader.h>

class DepthShader : public Shader {
public:
    [[nodiscard]] ShadedVertex PerVertex(const Vertex& vertex) override;
    [[nodiscard]] ShadedFragment PerFragment(const Fragment& frag) override;
};
