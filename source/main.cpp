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



int main() {

	constexpr int height{800};
	constexpr int width{800};
	
	PPMImage image{height,width};
	std::ofstream out_file{"image.ppm"};
    const auto white = std::array{1.f,1.f,1.f};

    Model head("assets/models/head.obj");

    //The vertex coordinates in the obj file are in [-1,1]. We need to scale it to the dimensions of our image

    for(const auto& face : head.Faces()) {
        for(int i =0; i<3; ++i) {
            Vec3 v0 = head.Vertices()[face[i]];
            Vec3 v1 = head.Vertices()[face[(i+1)%3]];

            int x0 = (v0.X() +1)*image.Width()/2;
            int y0 = ((-1.f*v0.Y()) +1)*image.Height()/2; //Flip the y coordinate

            int x1 = (v1.X() +1)*image.Width()/2;
            int y1 = ((-1.f*v1.Y()) +1)*image.Height()/2; //Flip the y coordinate

            DrawLine(x0,y0,x1,y1,image);

        }
    }

    

	if(!out_file) {std::cerr<<"Error creating file\n"; return 1;};
	image.Write(out_file);
}
