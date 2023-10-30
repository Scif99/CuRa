
#include <algorithm>
#include <cmath>
#include <chrono>
#include <iostream>
#include <memory>
#include <numeric>
#include <optional>
#include <span>
#include <vector>

#include "cura/buffer.h"
#include "cura/camera.h"
#include "cura/mat.h"
#include "cura/math.h"
#include "cura/model.h"
#include "cura/normal_map_shader.h"
#include "cura/pipeline.h"
#include "cura/rasterizer.h"
#include "cura/shader.h"
#include "cura/texture.h"
#include "cura/transforms.h"
#include "cura/triangle.h"
#include "cura/vec.h"

/*
OVERVIEW
for(const auto& entity: Scene) {
    std::vector<std::array<int,2> PixCoords = Project(entity);
    Rasterise(entity);
    Shade(entity, shading_model);
}
*/

//A vertex consists of a geometric position and a color
struct Vrtx {
    Vec2f pixel_coords;
    Color3f  col;
};


//Checks whether a pixel coordinate is occupied by a triangle
//If true, returns the barycentric coordinates at the pixel location
std::optional<std::array<float,3>> Opt_InTriangle(const Vec2f& p, const std::array<Vrtx, 3>& t) {
    const auto& [v0,v1,v2] = t; //Unpack vertices of triangle
    float w0 = EdgeFunction(v1.pixel_coords, v2.pixel_coords, p); // signed area of the triangle v1v2p multiplied by 2
    float w1 = EdgeFunction(v2.pixel_coords, v0.pixel_coords, p); // signed area of the triangle v2v0p multiplied by 2
    float w2 = EdgeFunction(v0.pixel_coords, v1.pixel_coords, p); // signed area of the triangle v0v1p multiplied by 2

    // Check if the pixel lies inside the triangle
    if (w0 >= 0 && w1 >= 0 && w2 >= 0) {
        float area = EdgeFunction(v0.pixel_coords, v1.pixel_coords, v2.pixel_coords); // The edge function with a triangles vertices as its arguments results in twice the area of triangle
        // Compute barycentric coordinates
        w0 /= area;
        w1 /= area;
        w2 /= area;
        return std::array{w0,w1,w2};
    }
    return std::nullopt;
}



//Determines visibility of a triangle and colors it using interpolation
void RasteriseAndColor(const std::array<Vrtx, 3>& t, Buffer<Color3f>& image) {
    for(int y =0; y < image.Height(); ++y ) {
        for(int x = 0; x<image.Width(); ++x) {
            auto p = Vec2f(x,y); //TODO move p to centre of pixel?
            // Check if the pixel lies inside the triangle
            if (auto bary_coords = Opt_InTriangle(p,t); bary_coords) {
                const auto& [w0,w1,w2] = bary_coords.value(); //unpack barycentric coordinates
                const auto& [v0,v1,v2] = t; //Unpack vertices of triangle
                
                //For all per-vertex attributes, we can now interpolate using barycentric coords (just color for now)
                //For example, the red component at pixel p is the weighted sum of the red components of t, weighted by barycentric coords
                //Note that v0.col is the RGB color of the 1st vertex of the triangle 
                //THIS IS CALLED GOURAD INTERPOLATION
                const auto r = w0*v0.col.R() + w1*v1.col.R() + w2*v2.col.R();
                const auto g = w0*v0.col.G() + w1*v1.col.G() + w2*v2.col.G();
                const auto b = w0*v0.col.B() + w1*v1.col.B() + w2*v2.col.B();
                auto col = Color3f{r,g,b};
                image.Set(x,y,col);
            }
        }
    }
}


#include <filesystem>
int main() {

	constexpr int height{800};
	constexpr int width{800};
	
	Buffer<Color3f> image{height,width};
	std::ofstream out_file{"/home/sc2046/Projects/Graphics/CuRa/scenes/01Triangle/triangle.ppm"};
    constexpr auto white = std::array{1.f,1.f,1.f};

    Model head("assets/models/head.obj");

    //Iterate over each face (triangle) in the model
    for(const auto& face : head.Faces()) {
        //Each face contains the indices of 3 vertices 
        //We extract the vertex positions using these indices
        const auto v0 = head.Vertices()[face.pos_idx[0]];
        const auto v1 = head.Vertices()[face.pos_idx[1]];
        const auto v2 = head.Vertices()[face.pos_idx[2]];

        //The positions are in 3D where each coord lies in [-1,1]
        //We need to extract the x and y coords and scale.
        Vec2f p0{(v0.X()+1)*width/2.f, (-v0.Y()+1)*height/2.f};
        Vec2f p1{(v1.X()+1)*width/2.f, (-v1.Y()+1)*height/2.f};
        Vec2f p2{(v2.X()+1)*width/2.f, (-v2.Y()+1)*height/2.f};

        //Define some random color for each vertex
        const auto col = Color3f{(float)rand() / (float)RAND_MAX,(float)rand() / (float)RAND_MAX,(float)rand() / (float)RAND_MAX} ;
        const Vrtx a{p0,col};
        const Vrtx b{p1,col};
        const Vrtx c{p2,col};

        const std::array<Vrtx, 3> triangle{a,b,c};

        RasteriseAndColor(triangle, image);
    }

	if(!out_file) {std::cerr<<"Error creating file\n"; return 1;};
	image.Write(out_file);
}
