#pragma once

#include <optional>

#include <cura/math.h>

/// @brief      Determines whether a point lies clockwise or anti-clockwise to a vector. 
/// @param tail Tail of the edge vector (i.e where the vector 'begins').
/// @param head Head of the edge vector (i.e. where the vector 'ends').
/// @param p    Point to be tested.
/// @return     Signed area of the parallelogram defined by the edge and the point. 
///             If the value is positive, then the point lies clockwise to the line.
///             If the value is negative, then the point lies anti-clockwise to the line.
///             If the value is zero, then the point lies on the line.
///             The value is used to compute barycentric coordinates.
[[nodiscard]] float EdgeFunction(const Vec2f& vfrom, const Vec2f& vto, const Vec2f& p) {

    assert(vfrom!=vto);
    return la::cross(vto - vfrom, p - vfrom);    
}


/// @brief Computes the barycentric coordinates of a point w.r.t. to a triangle, if possible. 
/// @param v0 First vertex of the triangle (in CCW).
/// @param v1 Second vertex of the triangle (in CCW).
/// @param v2 Third vertex of the triangle (in CCW).
/// @param p  Point to be tested.
/// @return   If the point lies inside the triangle contains the barycentric coordinates of the point. Otherwise is null.
[[nodiscard]] std::optional<Vec3f> oBarycentrics(const Vec2f& v0, const Vec2f& v1, const Vec2f& v2, const Vec2f& p) {
    
    auto w0{EdgeFunction(v1, v2, p)}; // signed area of the triangle v1v2p multiplied by 2
    auto w1{EdgeFunction(v2, v0, p)}; // signed area of the triangle v2v0p multiplied by 2
    auto w2{EdgeFunction(v0, v1, p)}; // signed area of the triangle v0v1p multiplied by 2

    // Check if the pixel lies inside the triangle
    //In the CCW winding case, this means the point should lie anti-clockwise to all directional edges of the triangle 
    if (w0 <= 0 && w1 <= 0 && w2 <= 0) {
         // The edge function with a triangles vertices as its arguments results in twice the area of triangle
        const auto area{EdgeFunction(v0, v1, v2)};
        
        // Compute barycentric coordinates
        w0 /= area;
        w1 /= area;
        w2 /= area;
        return Vec3f{w0,w1,w2};
    }
    return std::nullopt;
}
