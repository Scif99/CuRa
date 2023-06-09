#include <iostream>
#include "ppm_image.h"



/*
OVERVIEW
for(const auto& entity: Scene) {
    std::vector<std::array<int,2> PixCoords = Project(entity);
    Rasterise(entity);
    Shade(entity, shading_model);
}

*/

int main() {

    constexpr int height{256};
    constexpr int width{256};

    PPMImage image(height, width);
    std::ofstream out_file("image.ppm");
    image.Fill({1.f,0.f,0.f});
    image.Set(0,0,{0.f,1.f,0.f});
    image.Write(out_file);
}
