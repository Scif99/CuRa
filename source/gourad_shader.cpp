#include "gourad_shader.h"

ShadedVertex GouradShader::PerVertex(const Vertex& vertex) {
    //IN params
    auto& [aPos, aNormal, aTexCoord] = vertex; //unpack the vertex

    //UNIFORMS
    const auto model = GetUniform<Mat4f>("model");
    const auto view = GetUniform<Mat4f>("view");
    const auto projection =  GetUniform<Mat4f>("projection");

    //OUT params (sent to fragment shader)
    Vec4f gl_position;
    Color3f vertex_diffuse;

    const auto world_pos = model*Vec4f(aPos,1.f);
    gl_position =  projection*view*world_pos; //vertex position in clip space

    SetUniform("frag_pos", Cartesian(world_pos));

    //Compute diffuse component at the vertex
    const auto normal_transform =  Invert(Transpose(model)).value(); //TODO can this be indeterminate?
    const auto world_norm = Cartesian(normal_transform*Vec4f(aNormal,0.f));  //w is zero as normal should be unaffected by translations

    ShadedVertex out = {
        gl_position,
        UnitVector(world_norm),
        aTexCoord
    };
    return out; 
};  


ShadedFragment GouradShader::PerFragment(const Fragment& frag) {

    //INPUT
    const auto& [w_coords, norm, tex_coords] = frag;

    //UNIFORMS
    const auto light_dir = GetUniform<Norm3f>("light_dir");
    const auto light_ambient = GetUniform<Color3f>("light_ambient");
    const auto light_diffuse = GetUniform<Color3f>("light_diffuse");
    const auto light_specular = GetUniform<Color3f>("light_specular");
    const auto frag_pos  = GetUniform<Vec3f>("frag_pos");
    const auto view_pos = GetUniform<Vec3f>("view_pos");
    const auto model = GetUniform<Mat4f>("model"); //Need the model matrix again to transform the normal
    //OUTPUT
    ShadedFragment frag_data;
    frag_data.pos = w_coords;

    //const auto u_norm = UnitVector(norm);

    //ambient 
    Color3f ambient_component = light_ambient * TextureLookup(Texture("diffuse"), frag.tex_coords.U(),frag.tex_coords.V());

    //Normal
    Vec3f frag_norm = TextureLookup(Texture("normal"),frag.tex_coords.U(),frag.tex_coords.V());
    frag_norm = 2.f*(frag_norm - Vec3f(0.5f,0.5f,0.5f)); //Note the texture values are stored in the range[0,1] so we adjust to the range [-1,1]
    const auto normal_transform =  Invert(Transpose(model)).value(); //TODO can this be indeterminate?
    frag_norm = Cartesian(normal_transform*Vec4f(frag_norm,0.f));  //w is zero as normal should be unaffected by translations

    // diffuse 
    float diff = std::max(Dot(frag_norm, -light_dir),0.f);
    Color3f diffuse_component = light_diffuse * diff * TextureLookup(Texture("diffuse"), frag.tex_coords.U(),frag.tex_coords.V());

    // specular
    Vec3f viewDir = UnitVector(view_pos - frag_pos);
    Vec3f reflectDir = UnitVector(light_dir - 2*Dot(frag_norm,light_dir)*frag_norm);
    float spec = std::pow(std::max(Dot(viewDir, reflectDir), 0.f), 64);
    Color3f specular_component = light_specular * spec * TextureLookup(Texture("specular"), frag.tex_coords.U(),frag.tex_coords.V());  

    frag_data.FragColor = ambient_component+ diffuse_component + specular_component;

    return frag_data;
    };