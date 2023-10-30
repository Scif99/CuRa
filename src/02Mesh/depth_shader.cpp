#include <cura/depth_shader.h>


[[nodiscard]] ShadedVertex DepthShader::PerVertex(const Vertex& vertex) {

}


[[nodiscard]] ShadedFragment DepthShader::PerFragment(const Fragment& frag) {

    ShadedFragment shaded;
    shaded.pos = frag.window_coords;
    shaded.FragColor = Vec3f(shaded.pos.Z(),shaded.pos.Z(), shaded.pos.Z());
    return shaded;
}