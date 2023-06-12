#include <cmath>
#include <iostream>

#include "line.h"
#include "math.h"
#include "model.h"
#include "ppm_image.h"


/*
OVERVIEW
for(const auto& entity: Scene) {
    std::vector<std::array<int,2> PixCoords = Project(entity);
    Rasterise(entity);
    Shade(entity, shading_model);
}
*/

// Edge function used in triangle rasterisation
// Essentially computes the 2D cross product between an arbitrary point and a triangle edge
// If this sign is positive then the point lies in the triangle
// We keep the result for computing the barycentric coordinates
float EdgeFunction(int xp, int yp, int x0, int y0, int x1, int y1) {
    const auto ax = xp - x0;
    const auto ay = yp - y0;

    const auto bx = x1 - x0;
    const auto by = y1 - y0;

    return ax*by - ay*bx > 0;
}

using Triangle = std::array<std::array<int, 2>, 3>;
//Note that the vertices need to be specified in CCW order
bool InTriangle(int xp, int yp, const Triangle& t) {
    return EdgeFunction(xp,yp, t[0][0], t[0][1], t[1][0], t[1][1]) > 0 &&
           EdgeFunction(xp,yp, t[1][0], t[1][1], t[2][0], t[2][1]) > 0&& 
           EdgeFunction(xp,yp, t[2][0], t[2][1], t[0][0], t[0][1]) > 0;
}



void Rasterise(const Triangle& t, PPMImage& image) {
    for(int y =0; y < image.Height(); ++y ) {
        for(int x = 0; x<image.Width(); ++x) {
            if(InTriangle(x,y, t)) {
                image.Set(x,y,Color{1.f,0.f,0.f});
            }
        }
    }
}



int main() {

	constexpr int height{800};
	constexpr int width{800};
	
	PPMImage image{height,width};
	std::ofstream out_file{"image.ppm"};
    const auto white = std::array{1.f,1.f,1.f};


    auto v1 = std::array<int, 2>{200,200};
    auto v2 = std::array<int, 2>{600,600};
    auto v3 = std::array<int, 2>{600,0};
    Triangle triangle{v1,v2,v3};


    Rasterise(triangle, image);

    
    //The vertex coordinates in the obj file are in [-1,1]. We need to scale it to the dimensions of our image
    // Model head("assets/models/head.obj");
    // for(const auto& face : head.Faces()) {
    //     for(int i =0; i<3; ++i) {
    //         Vec3 v0 = head.Vertices()[face[i]];
    //         Vec3 v1 = head.Vertices()[face[(i+1)%3]];

    //         int x0 = (v0.X() +1)*image.Width()/2;
    //         int y0 = ((-1.f*v0.Y()) +1)*image.Height()/2; //Flip the y coordinate

    //         int x1 = (v1.X() +1)*image.Width()/2;
    //         int y1 = ((-1.f*v1.Y()) +1)*image.Height()/2; //Flip the y coordinate

    //         DrawLine(x0,y0,x1,y1,image);
    //     }
    // }

    

	if(!out_file) {std::cerr<<"Error creating file\n"; return 1;};
	image.Write(out_file);
}
