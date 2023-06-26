#ifndef SHADER_H
#define SHADER_H

#include <cassert>
#include <string>
#include <unordered_map>
#include <variant>

#include "buffer.h"
#include "vec.h"
#include "mat.h"
#include "vertex.h"


class Shader {
public:
    /// @brief Runs for each vertex. Must return at minimum the vertex position in clip coordinates.
    /// @param v_attributes 
    /// @return 
    virtual ProcessedVertex PerVertex(const VertexAttributes& v_attributes) = 0; 
    virtual FragmentData PerFragment(const Fragment& frag) = 0;


    //TODO add constraints
    //Uniform should be one of types inside the variant
    template<typename Uniform>
    void SetUniform(const std::string& name, Uniform f) { if(!uniforms.contains("name")) uniforms[name] = f; }   

    //TODO Handle errors
    template<typename Uniform>
    [[nodiscard]] Uniform GetUniform(const std::string& name) {
        assert(uniforms.contains(name)&& "Uniform not found");
        return std::get<Uniform>(std::visit([](auto&& arg) -> UniformType {return arg;}, uniforms[name]));
    }  

    //Store a ptr to a texture map
    void SetTexture(const std::string& name, const Buffer<Color3f>* p_tex) {textures[name] = p_tex;}

    //Retrieve a ptr to a texture map

    //TODO handle errors
    [[nodiscard]] const Buffer<Color3f>* Texture(const std::string& name) {
        assert(textures.contains(name)&& "Texture not found");
        return textures[name]; 
    }

private:
    using UniformType = std::variant<float, Vec2f, Vec3f, Norm3f, Mat4f>; //Contains all the possible types of a uniform variable

    std::unordered_map<std::string, UniformType> uniforms; //Models uniforms in opengl
    std::unordered_map<std::string, const Buffer<Color3f>*> textures; //Models sampler2D in opengl
};




//FLAT SHADING
//Norm3f triangle_norm{Cross<float,3>(Cartesian(world[1])-Cartesian(world[0]),Cartesian(world[2])-Cartesian(world[0]))};
//const float dot = Dot(triangle_norm ,-light.Direction); 
//vertices[i].diffuse = dot*light.Color;

#endif