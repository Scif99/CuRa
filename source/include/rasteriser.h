#ifndef RASTERISER_H
#define RASTERISER_H


#include <array>
#include <optional>

#include "triangle.h"
#include "vec.h"

// Edge function used in triangle rasterisation
// Essentially computes the 2D cross product between an arbitrary point and a triangle edge
// If this sign is positive then the point lies in the triangle
// We keep the result for computing the barycentric coordinates
[[nodiscard]] float EdgeFunction(const Vec2f& p, const Vec2f& v0, const Vec2f& v1) {

    assert(v0!=v1);

    auto a = p - v0; //Vector representing (ccw) edge of triangle
    auto b = v1 - v0; //Vector from vertex 0 to point

    return a.X()*b.Y() - a.Y()*b.X();
}


//Checks whether a pixel coordinate is occupied by a triangle
//If true, returns the barycentric coordinates at the pixel location
[[nodiscard]] std::optional<std::array<float,3>> Opt_InTriangle(const Vec2f& p, const Triangle& t) {

    //Extract 2D screen coordinates of each vertex
    const auto& v0 = Vec2f(t.vertices[0].clip_coords.X(),t.vertices[0].clip_coords.Y());
    const auto& v1 = Vec2f(t.vertices[1].clip_coords.X(),t.vertices[1].clip_coords.Y());
    const auto& v2 = Vec2f(t.vertices[2].clip_coords.X(),t.vertices[2].clip_coords.Y());

    auto l0{EdgeFunction(v1, v2, p)}; // signed area of the triangle v1v2p multiplied by 2
    auto l1{EdgeFunction(v2, v0, p)}; // signed area of the triangle v2v0p multiplied by 2
    auto l2{EdgeFunction(v0, v1, p)}; // signed area of the triangle v0v1p multiplied by 2

    // Check if the pixel lies inside the triangle
    if (l0 >= 0 && l1 >= 0 && l2 >= 0) {
         // The edge function with a triangles vertices as its arguments results in twice the area of triangle
        const auto area = float{EdgeFunction(v0, v1, v2)};
        // Compute barycentric coordinates
        l0 /= area;
        l1 /= area;
        l2 /= area;
        return std::array{l0,l1,l2};
    }
    return std::nullopt;
}


//Determines visibility of a triangle
//In terms of the pipeline, rakes a triangle primitive and breaks it down into fragments
//Interpolates any per-vertex attributes to get a value for the fragment
[[nodiscard]] std::vector<Fragment> Rasterise(const Triangle& triangle, const Buffer<Color3f>& image_buf) {

    std::vector<Fragment> triangle_frags; //The fragments that make up the triangle

    for(int y =0; y < image_buf.Height(); ++y ) {
        for(int x = 0; x <image_buf.Width(); ++x) {
            // Check if the pixel lies inside the triangle
            if (auto bary_coords = Opt_InTriangle(Point2f(x,y),triangle); bary_coords) {
                const auto& [l0,l1,l2] = bary_coords.value(); //unpack barycentric coordinates
                const auto& [v0,v1,v2] = triangle.vertices; //Unpack vertices of triangle

                Fragment frag;
                //Interpolate all per-vertex attributes
                const float frag_depth = l0*v0.clip_coords.Z() + l1*v1.clip_coords.Z() + l2*v2.clip_coords.Z();
                frag.window_coords = Vec3f(x,y,frag_depth);
                frag.tex_coords = l0*v0.tex_coords + l1*v1.tex_coords + l2*v2.tex_coords;
                frag.normal = l0*v0.world_norm + l1*v1.world_norm + l2*v2.world_norm;         

                triangle_frags.push_back(frag);
            }
        }
    }
    return triangle_frags;
}


#endif