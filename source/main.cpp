#include <cmath>
#include <iostream>

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

//Subroutine for drawing a line that mostly decreases in y 
void DrawNLine(int x0, int y0, int x1, int y1, PPMImage& image, const Color& col) {

    auto f = [x0,y0, x1, y1](float x, float y) {
            return (y0 - y1)*x + (x1-x0)*y + x0*y1 - x1*y0;
    };

    while(y0>=y1) {
        image.Set(x0,y0, col);
        auto shared_midpoint_x{x0+ 0.5f};
        auto shared_midpoint_y{y0-1.f};

        //If the line passes below the midpoint, we also move along x
        if(f(shared_midpoint_x,shared_midpoint_y) < 0) { ++x0;}
        --y0;
    }
}

//Subroutine for drawing a line that moves mostly increases in x but slightly decreases in y
void DrawNELine(int x0, int y0, int x1, int y1, PPMImage& image, const Color& col) {

    auto f = [x0,y0, x1, y1](float x, float y) {
            return (y0 - y1)*x + (x1-x0)*y + x0*y1 - x1*y0;
    };

    while(x0<=x1) {
        image.Set(x0,y0, col);
        auto shared_midpoint_x{x0+ 1.f};
        auto shared_midpoint_y{y0-0.5f};

        //If line passes above midpoint, we also move along y
        if(f(shared_midpoint_x,shared_midpoint_y) > 0) { --y0;}
        ++x0;
    }
}


//Subroutine for drawing a line that moves mostly increases in x but slightly increases in y
void DrawSELine(int x0, int y0, int x1, int y1, PPMImage& image, const Color& col) {

    auto f = [x0,y0, x1, y1](float x, float y) {
            return (y0 - y1)*x + (x1-x0)*y + x0*y1 - x1*y0;
    };

    while(x0<=x1) {
        image.Set(x0,y0, col);
        auto shared_midpoint_x{x0+ 1.f};
        auto shared_midpoint_y{y0-0.5f};

        //If the line passes below the midpoint, we also move along y
        if(f(shared_midpoint_x,shared_midpoint_y) < 0) { ++y0;}
        ++x0;
    }
}

//Subroutine for drawing a line that mostly increases in y 
void DrawSLine(int x0, int y0, int x1, int y1, PPMImage& image, const Color& col) {

    auto f = [x0,y0, x1, y1](float x, float y) {
            return (y0 - y1)*x + (x1-x0)*y + x0*y1 - x1*y0;
    };

    while(y0<=y1) {
        image.Set(x0,y0, col);
        auto shared_midpoint_x{x0 + 0.5f};
        auto shared_midpoint_y{y0+1.f};

        //Line passes above the midpoint, then we also move along x
        if(f(shared_midpoint_x,shared_midpoint_y) > 0) { ++x0;}
        ++y0;
    }
}


/// @brief Draws a line from one pixel to another. Implemented using the midpoint algorithm.
/// @brief The idea is that we move from left to right, and at each step we decide whether to move up/down or not.
/// @brief This decision is based upon the value of f evaluated at the shared edge of the candidate pixels
/// @param x0 Initial x coordinate
/// @param y0 Initial y coordinate
/// @param x1 Final x coordinate
/// @param y1 Final y coordinates
/// @param image The image that the line is drawn on
void DrawLine(int x0, int y0, int x1, int y1, PPMImage& image) {

    Color white{1.f,1.f,1.f};

    //First evaluate special cases (horizontal/vertical) lines
    //Horizontal
    if(y0==y1) {
        while(x0<=x1) {
            image.Set(x0,y0,white);
            ++x0;
        }
        return;
    }

    //Vertical
    if(x0==x1) {
        if(y1<y0) {std::swap(y0,y1);} //want to be increasing in y
        while(y0<=y1) {
            image.Set(x0,y0,white);
            ++y0;
        }
        return;
    }

    //Although the idea of the algorithm is the same in all cases, there are some slight differences depending on the gradient of the line.
    //Remember that the y axis points downwards here...
    //We want the gradient to be defined starting from the leftmost (smalller x) point

    if(x1<x0) {
        std::swap(x0,x1);
        std::swap(y0,y1);
    }
    const auto grad = float{static_cast<float>(y1-y0)/static_cast<float>(x1-x0)};
    if(grad<-1.f) {
        DrawNLine(x0, y0, x1, y1, image, white);
    }

    else if(grad>=-1.f && grad<0.f) {
        DrawNELine(x0, y0, x1, y1, image, white);
    }

    else if(grad>0.f && grad<=1.f) {
        DrawSELine(x0, y0, x1, y1, image, white);

    }

    else {
        DrawSLine(x0, y0, x1, y1, image, white);
    }

}

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
