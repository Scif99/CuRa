
#include <iostream>
#include <optional>
#include <vector>

#include <linalg.h>

#include "cura/buffer.h"


using vec2i = linalg::vec<int,2>; 

[[nodiscard]] float EdgeFunction(const vec2i& vtail, const vec2i& vhead, const vec2i& p) {

    assert(vtail!=vhead);

    const auto b = vhead - vtail; //Vector from start of edge to point  
    const auto a = p - vtail; //Vector representing (ccw) edge of triangle
    //return a x b
    return a.x*b.y - a.y*b.x;
}

//Determines visibility of a triangle and colors it using interpolation
void DrawTriangle(const vec2i& v0,const vec2i& v1,const vec2i& v2, Buffer<Color3f>& image) {

    //Determine bounding box of triangle
    int minX = std::min({v0.x,v1.x, v2.x}); 
    int maxX = std::max({v0.x,v1.x, v2.x}); 

    int minY = std::min({v0.y,v1.y, v2.y}); 
    int maxY = std::max({v0.y,v1.y, v2.y}); 

    // Dont need to draw anything outside screen bounds
    minX = std::max(minX, 0);
    minY = std::max(minY,0);

    maxX = std::min(maxX, image.Width());
    maxY = std::min(maxY, image.Height());

    for(int y = minY; y < maxY; ++y )
    {
        for(int x = minX; x< maxX; ++x) 
        {
            const auto p = vec2i(x,y); //Current pixel being tested

            const float w0 = EdgeFunction(v1, v2, p); // signed area of the triangle v1v2p multiplied by 2
            const float w1 = EdgeFunction(v2, v0, p); // signed area of the triangle v2v0p multiplied by 2
            const float w2 = EdgeFunction(v0, v1, p); // signed area of the triangle v0v1p multiplied by 2

            if(w0>=0 && w1>=0 && w2>=0) 
            {
                image.Set(x,y,Color3f(1.f,0.f,0.f));
            }
        }
    }
}


int main() {

	constexpr int kheight{800};
	constexpr int kwidth{800};
    constexpr auto kblack = Color3f{0.f,0.f,0.f};	
	Buffer<Color3f> image{kheight,kwidth, kblack};
	std::ofstream out_file{"/home/sc2046/Projects/Graphics/CuRa/scenes/01Triangle/triangle.ppm"};

    DrawTriangle(
        vec2i{0,0},
        vec2i{0,200},
        vec2i{200,200},
        image
        );

	if(!out_file) {std::cerr<<"Error creating file\n"; return 1;};
	image.Write(out_file);
}
