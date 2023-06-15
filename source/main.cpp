#include <cmath>
#include <iostream>
#include <memory>
#include <optional>

#include "line.h"
#include "math.h"
#include "model.h"
#include "ppm_image.h"
#include "vec.h"


/*
OVERVIEW
for(const auto& entity: Scene) {
    std::vector<std::array<int,2> PixCoords = Project(entity);
    Rasterise(entity);
    Shade(entity, shading_model);
}
*/

//A vertex consists of a geometric position and a color
struct Vertex {
    Vec2 pixel_coords;
    Color3  col;
};

//Note that the vertices need to be specified in CCW order
using Triangle = std::array<Vertex, 3>;


//Checks whether a pixel coordinate is occupied by a triangle
//If true, returns the barycentric coordinates at the pixel location
std::optional<std::array<float,3>> Opt_InTriangle(const Vec2& p, const Triangle& t) {
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
void RasteriseAndColor(const Triangle& t, PPMImage& image) {
    for(int y =0; y < image.Height(); ++y ) {
        for(int x = 0; x<image.Width(); ++x) {
            auto p = Vec2(x,y); //TODO move p to centre of pixel?
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
                auto col = Color3{r,g,b};
                image.Set(x,y,col);

            }
        }
    }
}



int main() {

	constexpr int height{8};
	constexpr int width{8};
	
	PPMImage image{height,width};
	std::ofstream out_file{"image.ppm"};
    constexpr auto white = std::array{1.f,1.f,1.f};

    Model head("assets/models/head.obj");

    //Iterate over each face (triangle) in the model
    for(const auto& face : head.Faces()) {
        //Each face contains the indices of 3 vertices 
        //We extract the vertex positions using these indices
        const auto v0 = head.Vertices()[face[0]];
        const auto v1 = head.Vertices()[face[1]];
        const auto v2 = head.Vertices()[face[2]];

        //The positions are in 3D where each coord lies in [-1,1]
        //We need to extract the x and y coords and scale.
        Vec2 p0{(v0.X()+1)*width/2.f, (-v0.Y()+1)*height/2.f};
        Vec2 p1{(v1.X()+1)*width/2.f, (-v1.Y()+1)*height/2.f};
        Vec2 p2{(v2.X()+1)*width/2.f, (-v2.Y()+1)*height/2.f};

        //Define some random color for each vertex
        const auto col = Color3{(float)rand() / (float)RAND_MAX,(float)rand() / (float)RAND_MAX,(float)rand() / (float)RAND_MAX} ;
        const Vertex a{p0,col};
        const Vertex b{p1,col};
        const Vertex c{p2,col};

        const Triangle triangle{a,b,c};

        RasteriseAndColor(triangle, image);
    }

	if(!out_file) {std::cerr<<"Error creating file\n"; return 1;};
	image.Write(out_file);
}
