#ifndef MATH_H
#define MATH_H

#include "vec.h"
#include <vector>

/// @brief Evaluates the implicit equation of a line at a particular point.
/// @brief The point lies on the line if the return value is zero.
/// @brief Assumes x0<x1
/// @param x_at x coordinate to evaluate at
/// @param y_at y coordinate to evaluate at
/// @return Value of the equation evaluated at (x_at,y_at). Is zero if the point lies on the line.
inline float ImplicitLine(float x0, float y0, float x1, float y1, float x_at, float y_at) {
    //assert(x0<x1);
    return (y0 - y1)*x_at + (x1-x0)*y_at + x0*y1 - x1*y0;
}

// Edge function used in triangle rasterisation
// Essentially computes the 2D cross product between an arbitrary point and a triangle edge
// If this sign is positive then the point lies in the triangle
// We keep the result for computing the barycentric coordinates
float EdgeFunction(const Vec2& p, const Vec2& v0, const Vec2& v1) {
    auto a = p - v0; //Vector representing (ccw) edge of triangle
    auto b = v1 - v0; //Vector from vertex 0 to point

    return a.X()*b.Y() - a.Y()*b.X();
}

#endif