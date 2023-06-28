#ifndef GOURAD_H
#define GOURAD_H

#include "shader.h"

class GouradShader : public Shader {
public:
    [[nodiscard]] ShadedVertex PerVertex(const Vertex& vertex) override;
    [[nodiscard]] ShadedFragment PerFragment(const Fragment& frag) override;
};

#endif