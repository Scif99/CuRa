#pragma once

#include "shader.h"
#include "vec.h"

class QuadShader : public Shader {
public:
    [[nodiscard]] ShadedVertex PerVertex(const Vertex& vertex) override;
    [[nodiscard]] ShadedFragment PerFragment(const Fragment& frag) override;

};

inline ShadedVertex QuadShader::PerVertex(const Vertex& vertex) {
    //IN params
    auto& [aPos, aNormal, aTexCoord] = vertex; //unpack the vertex

    //UNIFORMS
    const auto model = GetUniform<Mat4f>("model");
    const auto view = GetUniform<Mat4f>("view");
    const auto projection =  GetUniform<Mat4f>("projection");



    //OUT params (Interpolated during rasterisation then sent to fragment shader)
    Vec4f gl_position = projection*view*model*Vec4f(aPos,1.f);
    Vec3f vertex_world_normal;

    ShadedVertex out = {
        gl_position,
        vertex_world_normal,
        aTexCoord
    };
    return out; 
};  



inline ShadedFragment QuadShader::PerFragment(const Fragment& frag) {

    //INPUT
    const auto& [w_coords, norm, tex_coords] = frag;

    ShadedFragment frag_data;
    frag_data.pos = w_coords;
    frag_data.FragColor = Color3f(1.f,0.f,0.f);
    return frag_data;
    };
