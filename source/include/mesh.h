#include <vector>

#include "shader.h"
#include "vec.h"


class Mesh {

    std::vector<Vec3f> vertices_;
    std::vector<Vec2f> tex_coords_;
    std::vector<Vec3f> normals_;

    void Draw(const Shader& shader);
};

void Mesh::Draw(const Shader& shader) {
    


}