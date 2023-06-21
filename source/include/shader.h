#ifndef SHADER_H
#define SHADER_H

#include "vec.h"
/*
VERTEX SHADER
- Applies transforms to clip space
- Does any per-vertex lighting
*/

class ShaderInterface {
    virtual void PerVertex(const Vec3f& v) = 0;
    virtual void PerFragment() = 0;
};

class FlatShader : ShaderInterface{

    void PerVertex(const Vec3f& v) override;
};

void FlatShader::PerVertex(const Vec3f& v, const Mat4f& MVP) {

    Vec4f gl_position = Vec4f(v,1.f);

}