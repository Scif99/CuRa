#ifndef MESH_H
#define MESH_H

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
};

#endif 