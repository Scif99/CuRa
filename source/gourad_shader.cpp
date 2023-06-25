#include "gourad_shader.h"
#include <iostream>
ProcessedVertex GouradShader::PerVertex(const VertexAttributes& v_attributes) {
    //IN params
    auto& [aPos, aNormal, aTexCoord] = v_attributes; //unpack the vertex

    //UNIFORMS
    const auto model = GetUniform<Mat4f>("model");
    const auto view = GetUniform<Mat4f>("view");
    const auto projection =  GetUniform<Mat4f>("projection");
    const auto light_color = GetUniform<Color3f>("light_color");
    const auto light_dir = GetUniform<Norm3f>("light_dir");

    //OUT params (sent to fragment shader)
    Vec4f gl_position;
    Color3f vertex_diffuse;

    const auto world_pos = model*Vec4f(aPos,1.f); //vertex position in world space
    gl_position =  projection*view*world_pos; //vertex position in clip space

    //Compute diffuse component at the vertex
    const auto normal_transform = Invert(Transpose(model)).value(); //TODO can this be indeterminate?
    const auto world_norm = normal_transform*Vec4f(aNormal,0.f); //NOTE we set the w coordinate to 0 to ignore any translational components
    const float dot = Dot(UnitVector(Cartesian(world_norm)) , UnitVector(-light_dir)); //Angle between light direction and vertex normal
    vertex_diffuse = dot*light_color;

    ProcessedVertex out = {
        gl_position,
        aTexCoord,
        vertex_diffuse
    };
    return out; 
};  


FragmentData GouradShader::PerFragment(const Fragment& frag) {

       FragmentData frag_data;

       frag_data.pos = frag.window_coords;

       const Buffer<Color3f>* tex = Texture("diffuse");
       frag_data.FragColor = tex->Get(frag.tex_coords.U(), frag.tex_coords.V());
       frag_data.FragColor = frag_data.FragColor;//*frag.diffuse;


       return frag_data;
    };