#ifndef LINE_H
#define LINE_H

#include "buffer.h"
#include "math.h"
#include "ppm_image.h"
#include "vec.h"

//Subroutine for drawing a line that mostly decreases in y 
inline void DrawNLine(int x0, int y0, int x1, int y1, Buffer<Color3f>& image, const Color3f& col) {

    auto x_curr{x0};
    auto y_curr{y0};

    while(y_curr>=y1) {
        image.Set(x_curr,y_curr, col);
        auto shared_midpoint_x{x_curr+ 0.5f};
        auto shared_midpoint_y{y_curr-1.f};

        //If the line passes below the midpoint, we also move along x
        if(ImplicitLine(x0,y0,x1,y1,shared_midpoint_x,shared_midpoint_y) < 0) { ++x_curr;}
        --y_curr;
    }
}

//Subroutine for drawing a line that moves mostly increases in x but slightly decreases in y
inline void DrawNELine(int x0, int y0, int x1, int y1, Buffer<Color3f>& image, const Color3f& col) {

    auto x_curr{x0};
    auto y_curr{y0};

    while(x_curr<=x1) {
        image.Set(x_curr,y_curr, col);
        auto shared_midpoint_x{x_curr+ 1.f};
        auto shared_midpoint_y{y_curr-0.5f};

        //If line passes above midpoint, we also move along y
        if(ImplicitLine(x0,y0,x1,y1,shared_midpoint_x,shared_midpoint_y) > 0) { --y_curr;}
        ++x_curr;
    }
}


//Subroutine for drawing a line that moves mostly increases in x but slightly increases in y
inline void DrawSELine(int x0, int y0, int x1, int y1, Buffer<Color3f>& image, const Color3f& col) {

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
inline void DrawSLine(int x0, int y0, int x1, int y1, Buffer<Color3f>& image, const Color3f& col) {

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
inline void DrawLine(int x0, int y0, int x1, int y1, Buffer<Color3f>& image) {

    Color3f white{1.f,1.f,1.f};

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


#endif 