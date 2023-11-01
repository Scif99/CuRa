#pragma once

#include <array>
#include <optional>
#include <unordered_map>
#include <vector>

#include <cura/math.h>
#include <cura/vertex.h>



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

// Determines visibility of a triangle
// In terms of the pipeline, rakes a triangle primitive and breaks it down into fragments
// Interpolates any per-vertex attributes to get a value for the fragment
//  void Rasterise(const Vec2f& v0, const Vec2f& v1, const Vec2f& v2 , FrameBuffer& buffer) {
    
//     //assert(attribs.contains("Depth"))
    
//     std::vector<Fragment> triangle_frags; //The fragments that make up the triangle

//     //Determine bounding box of triangle
//     int minX = std::min({v0.x,v1.x, v2.x}); 
//     int maxX = std::max({v0.x,v1.x, v2.x}); 
//     int minY = std::min({v0.y,v1.y, v2.y}); 
//     int maxY = std::max({v0.y,v1.y, v2.y}); 

//     // Dont need to draw anything outside screen bounds
//     minX = std::max(minX, 0);
//     minY = std::max(minY,0);
//     maxX = std::min(maxX, buffer.width);
//     maxY = std::min(maxY, buffer.height);

//     for(int y = minY; y <= maxY; ++y ) {
//         for(int x = minX;  x <= maxX; ++x) {
//             const auto p = Vec2f(x,y);
//             // Check if the pixel lies inside the triangle
//             if (auto bary_coords = oBarycentrics(v0,v1,v2,p); bary_coords) {
//                 const auto& [l0,l1,l2] = bary_coords.value(); //unpack barycentric coordinates

//                 //Interpolate all per-vertex attributes
//                 for(const auto& att) {

//                     buffer.Color(x,y) = att->BaryInterp(l0,l1,l2);
//                 }
//             }
//         }
//     }
//     //return triangle_frags;
// }


//TODO
// bool IsFrontFacing(const Vec2f& v0, const Vec2f& v1, const Vec3f& v2, bool CCWWinding = true) {
//     return true;
// }