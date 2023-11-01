#include <iostream>

#include <cura/buffer.h>
#include <cura/math.h>

[[nodiscard]] float EdgeFunction(const Vec2f& vfrom, const Vec2f& vto, const Vec2f& p) {

    assert(vfrom!=vto);
    return la::cross(vto - vfrom, p - vfrom);    
}

//Determines visibility of a triangle and colors it using interpolation
void DrawTriangle(const Vec2f& v0,const Vec2f& v1,const Vec2f& v2, FrameBuffer& image) {

    Color3f triangleCol = Color3f(
                static_cast <float> (rand()) / static_cast <float> (RAND_MAX),
                static_cast <float> (rand()) / static_cast <float> (RAND_MAX),
                static_cast <float> (rand()) / static_cast <float> (RAND_MAX)
                );

    //Determine bounding box of triangle
    std::int32_t minX = std::min({v0.x,v1.x, v2.x}); 
    std::int32_t maxX = std::max({v0.x,v1.x, v2.x}); 

    std::int32_t minY = std::min({v0.y,v1.y, v2.y}); 
    std::int32_t maxY = std::max({v0.y,v1.y, v2.y}); 

    // Dont need to draw anything outside screen bounds
    minX = std::max(minX, 0);
    minY = std::max(minY,0);

    maxX = std::min(maxX, image.width);
    maxY = std::min(maxY, image.height);

    for(auto y = minY; y < maxY; ++y )
    {
        for(auto x = minX; x< maxX; ++x) 
        {
            const auto p = Vec2f(x,y); //Current pixel being tested

            const float w0 =  EdgeFunction(v1, v2, p); // signed area of the triangle v1v2p multiplied by 2
            const float w1 =  EdgeFunction(v2, v0, p); // signed area of the triangle v2v0p multiplied by 2
            const float w2 =  EdgeFunction(v0, v1, p); // signed area of the triangle v0v1p multiplied by 2

            //Note that the comparitor (<= vs >=) depends on the winding order (CCW vs CW)!
            if(w0<=0 && w1<=0 && w2<=0) 
            {
                image.Color(x,y) = triangleCol;
            }
        }
    }
}


//Draws a single triangle.
int main() {

	constexpr int kheight{800};
	constexpr int kwidth{800};
	FrameBuffer image{kheight,kwidth};
	std::ofstream out_file{"/home/sc2046/Projects/Graphics/CuRa/scenes/01Triangle/triangle.ppm"};


     DrawTriangle(
         Vec2f{0.f,0.f},
         Vec2f{0.f,200.f},
         Vec2f{200.f,200.f},
         image
        );

	if(!out_file) {std::cerr<<"Error creating file\n"; return 1;};
	image.WriteColorsPPM(out_file);
}
