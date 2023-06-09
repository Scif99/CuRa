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

/// @brief Coordinates are range [0,1]
/// @param x0 Initial x coordinate
/// @param y0 Initial y coordinate
/// @param x1 Final x coordinate
/// @param y1 Final y coordinates
/// @param image The image that the line is drawn on
void DrawLine(float x0, float y0, float x1, float y1, PPMImage& image) {
    auto t = 0.f;
    while(t<=1.f) {
        auto x_lerp = image.Width() * ((1.f-t)*x0 + t*x1);
        auto y_lerp = image.Height() * ((1.f-t)*y0 + t*y1);

        image.Set(static_cast<int>(y_lerp) ,static_cast<int>(x_lerp), {1.f,1.f,1.f}); //Set lerped pixel to white
        t+=0.01f;
    }
}

int main() {

    constexpr int height{256};
    constexpr int width{256};

    PPMImage image(height, width);
    std::ofstream out_file("image.ppm");
    DrawLine(0.f,0.5f,0.5f,0.5f,image); //Draw a line from (0,0.5) to (0.5,0.5)
    image.Write(out_file);
}
