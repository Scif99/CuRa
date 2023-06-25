#ifndef SHADER_H
#define SHADER_H

#include <string>
#include <unordered_map>
#include <variant>
#include <string_view>

#include "vec.h"
#include "mat.h"
#include "vertex.h"



class Shader {
public:

    /// @brief Input: Vertex attributes in local space
    /// @brief Output: Vertex positions in clip space
    virtual void PerVertex(const Vertex& vertex) = 0; 
    virtual void PerFragment() = 0;
};


class GouradShader : public Shader {
public:
    using UniformType = std::variant<float, Vec2f, Vec3f, Norm3f, Mat4f>;

    void PerVertex(const Vertex& v) override {
    //IN params
    const auto& [aPos, aNormal, aTexCoord] = v; //unpack the vertex

    //OUT params (sent to fragment shader)
    Vec4f gl_position;
    Color3f vertex_diffuse;

    //UNIFORMS
    auto model = GetUniform<Mat4f>("model");
    auto view = GetUniform<Mat4f>("view");
    auto projection =  GetUniform<Mat4f>("projection");
    auto light_color = GetUniform<Color3f>("light_color");
    auto light_dir = GetUniform<Vec3f>("light_dir");

    const auto world_pos = model*Vec4f(aPos,1.f); //vertex position in world space
    gl_position =  projection*view*world_pos; //vertex position in clip space

    //Compute diffuse color at the vertex

    const auto normal_transform = Invert(Transpose(model)).value();
    const auto world_norm = normal_transform*Vec4f(aNormal,0.f); //NOTE we set the w coordinate to 0 to ignore any translational components
    const float dot = Dot(UnitVector(Cartesian(world_norm)) , -UnitVector(light_dir));
    vertex_diffuse = dot*light_color;

        
    };   

    void PerFragment() override {};

    
    //TODO add constraints
    //Uniform should be a type contained inside variant
    template<typename Uniform>
    void SetUniform(const std::string& name, Uniform f) {
        uniforms[name] = f;
    }   

    //TODO Handle errors
    template<typename Uniform>
    Uniform GetUniform(const std::string& name) {

        return std::get<Uniform>(std::visit([](auto&& arg) -> UniformType {return arg;}, uniforms[name]));
    }  

public:
    Vec4f gl_position;
    Color3f vertex_diffuse;

private:
    std::unordered_map<std::string, UniformType> uniforms;
};

#endif