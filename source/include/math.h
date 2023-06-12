#ifndef MATH_H
#define MATH_H

#include "vec.h"
#include <vector>

/// @brief Evaluates the implicit equation of a line at a particular point.
/// @brief The point lies on the line if the return value is zero.
/// @brief Assumes x0<x1
/// @param x0 x coordinate at the start of the line segment
/// @param y0 y coordinate at the start of the line segment
/// @param x1 x coordinate at the end of the line segment
/// @param y1 y coordinate at the end of the line segment
/// @param x_at x coordinate to evaluate at
/// @param y_at y coordinate to evaluate at
/// @return Value of the equation evaluated at (x_at,y_at). Is zero if the point lies on the line.
inline float ImplicitLine(float x0, float y0, float x1, float y1, float x_at, float y_at) {
    //assert(x0<x1);
    return (y0 - y1)*x_at + (x1-x0)*y_at + x0*y1 - x1*y0;
}



#endif