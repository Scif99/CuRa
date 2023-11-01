
#include <iostream>
#include <vector>

#include <cura/buffer.h>
#include <cura/math.h>
#include <cura/model.h>

[[nodiscard]] float EdgeFunction(const Vec2f& vfrom, const Vec2f& vto, const Vec2f& p) {

    assert(vfrom!=vto);
    return la::cross(vto - vfrom, p - vfrom);    
}

//This function is exactly the same as in the previous scene.
void DrawTriangle(const Vec2f& v0,const Vec2f& v1,const Vec2f& v2, FrameBuffer& image) {

    Color3f triangleCol = Color3f(
                    static_cast <float> (rand()) / static_cast <float> (RAND_MAX),
                    static_cast <float> (rand()) / static_cast <float> (RAND_MAX),
                    static_cast <float> (rand()) / static_cast <float> (RAND_MAX));

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

    static int r = 0;

    for(auto y = minY; y <= maxY; ++y )
    {
        for(auto x = minX; x <= maxX; ++x) 
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

int main() {

	constexpr int kheight{800};
	constexpr int kwidth{800};

	FrameBuffer image{kheight,kwidth};
	std::ofstream out_file{"/home/sc2046/Projects/Graphics/CuRa/scenes/02Mesh/face_mesh.ppm"};


    Model head("/home/sc2046/Projects/Graphics/CuRa/assets/models/head.obj");

    //Iterate over each face (triangle) in the model
    for(const auto& face : head.Faces()) {
        //Each face contains the indices of 3 vertices 
        //We extract the vertex positions using these indices
        const auto v0 = head.Vertices()[face.pos_idx[0]];
        const auto v1 = head.Vertices()[face.pos_idx[1]];
        const auto v2 = head.Vertices()[face.pos_idx[2]];

        //The positions are in 3D where each coord lies in [-1,1]
        //We need to extract the x and y coords and scale.
        Vec2f p0{(v0.x+1)*kwidth/2.f, (-v0.y+1)*kheight/2.f};
        Vec2f p1{(v1.x+1)*kwidth/2.f, (-v1.y+1)*kheight/2.f};
        Vec2f p2{(v2.x+1)*kwidth/2.f, (-v2.y+1)*kheight/2.f};

        DrawTriangle(p0,p1,p2, image);
    }

	if(!out_file) {std::cerr<<"Error creating file\n"; return 1;};
	image.WriteColorsPPM(out_file);
}