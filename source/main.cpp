#include <cmath>
#include <iostream>

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

// Edge function used in triangle rasterisation
// Essentially computes the 2D cross product between an arbitrary point and a triangle edge
// If this sign is positive then the point lies in the triangle
// We keep the result for computing the barycentric coordinates
float EdgeFunction(const Vec2& p, const Vec2& v0, const Vec2& v1) {
    auto a = p - v0; //Vector representing (ccw) edge of triangle
    auto b = v1 - v0; //Vector from vertex 0 to point

    auto c = -a;
    c+=b;

    return a.X()*b.Y() - a.Y()*b.X() > 0;
}

using Triangle = std::array<Vec2, 3>;
//Note that the vertices need to be specified in CCW order
bool InTriangle(const Vec2& p, const Triangle& t) {
    return EdgeFunction(p, t[0], t[1]) > 0 &&
           EdgeFunction(p, t[1], t[2]) > 0&& 
           EdgeFunction(p,t[2],t[0]) > 0;
}



void Rasterise(const Triangle& t, PPMImage& image) {
    for(int y =0; y < image.Height(); ++y ) {
        for(int x = 0; x<image.Width(); ++x) {
            if(InTriangle(Vec2(x,y), t)) {
                image.Set(x,y,Color3{1.f,0.f,0.f});
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


    auto v1 = Vec2{200,200};
    auto v2 = Vec2{600,600};
    auto v3 = Vec2{600,0};
    Triangle triangle{v1,v2,v3};


    Rasterise(triangle, image);
	if(!out_file) {std::cerr<<"Error creating file\n"; return 1;};
	image.Write(out_file);
}
