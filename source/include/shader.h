#ifndef SHADER_H
#define SHADER_H

#include <cassert>
#include <concepts>
#include <mutex>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <variant>

#include "buffer.h"
#include "vec.h"
#include "mat.h"
#include "vertex.h"

inline Color3f TextureLookup( const Buffer<Color3f>* texture, float u, float v, bool flip_v = true) {
    assert(u>=0 && u<=1.f);
    assert(v>=0 && v<=1.f);

    //Also need to scale texture coordinates to the texture's dimensions
    const auto tw{texture->Width()};
    const auto th{texture->Height()}; 

    const float scaled_u = u*tw;
    const float scaled_v = flip_v ? th - v*th : v*th;
    return texture->Get(scaled_u,scaled_v);
}

using Uniform = std::variant<float, Vec2f, Vec3f, Norm3f, Mat4f>; //Contains all the possible types of a uniform variable

//A trait used to determine whether a type matches with at least one from some list of types
template<typename T, typename Args> struct is_one_of {};
template<typename T, typename... Args> 
struct is_one_of<T,std::variant<Args...>> : std::bool_constant<(std::is_same_v<T,Args> || ...)> {};

template<typename T>
concept UniformType = is_one_of<T,Uniform>::value;

class Shader {
public:

    virtual ~Shader() {}
    // Runs for each vertex. Must return at minimum the vertex position in clip coordinates.
    virtual ShadedVertex PerVertex(const Vertex& vertex) = 0; 
    // Runs for each fragment rasterised from a triangle. 
    virtual ShadedFragment PerFragment(const Fragment& frag) = 0;


    template<UniformType T>
    void SetUniform(const std::string& name, T f) { 
        //const std::lock_guard<std::mutex> lock(mtx);
            uniforms_[name] = f; 
    }   

    template<UniformType T>
    [[nodiscard]] T GetUniform(const std::string& name) {
        //const std::lock_guard<std::mutex> lock(mtx);
        assert(uniforms_.contains(name)&& "Uniform not found");
        return std::get<T>(std::visit([](auto&& arg) -> Uniform {return arg;}, uniforms_[name])); //Note the lambda returns an std::variant
    }  

    //Store a ptr to a texture map
    void SetTexture(const std::string& name, const Buffer<Color3f>* p_tex) {
        //const std::lock_guard<std::mutex> lock(mtx);
        textures_[name] = p_tex;
    }
    
    //Retrieve a ptr to a texture map
    //TODO handle errors
    [[nodiscard]] const Buffer<Color3f>* Texture(const std::string& name) {
        //const std::lock_guard<std::mutex> lock(mtx);
        assert(textures_.contains(name)&& "Texture not found");
        return textures_[name]; 
    }

private:

    std::unordered_map<std::string, Uniform> uniforms_; //Models uniforms in opengl
    std::unordered_map<std::string, const Buffer<Color3f>*> textures_; //Models sampler2D in opengl
    //std::mutex mtx;
};




//FLAT SHADING
//Norm3f triangle_norm{Cross<float,3>(Cartesian(world[1])-Cartesian(world[0]),Cartesian(world[2])-Cartesian(world[0]))};
//const float dot = Dot(triangle_norm ,-light.Direction); 
//vertices[i].diffuse = dot*light.Color;

#endif