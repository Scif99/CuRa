#pragma once


#include <string>
#include <unordered_map>
#include <vector>

#include "buffer.h"
#include "vec.h"
#include "vertex.h"

struct Mesh {

    std::vector<Vertex> vertices_;
    std::vector<int> indices_;
    std::unordered_map<std::string, Buffer<Color3f>* > textures_;

    void Draw(Buffer<Color3f>& framebuffer);
};

void Mesh::Draw(Buffer<Color3f>& framebuffer) {

    //1) Pass vertices through vertex shader

    //2) Primitive assembly

    //3) Clipping
    //4) Face Culling

    //5) Rasterise
}
