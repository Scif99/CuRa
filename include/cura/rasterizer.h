#pragma once

#include <array>
#include <optional>


#include "triangle.h"
#include "vec.h"

// Returns a positive value if p lies to the right of the edge vector
// The value of the result is used in computing barycentric coordinates
[[nodiscard]] float EdgeFunction(const Vec2f& vtail, const Vec2f& vhead, const Vec2f& p) {

    assert(vtail!=vhead);

    const auto b = vhead - vtail; //Vector from start of edge to point  
    const auto a = p - vtail; //Vector representing (ccw) edge of triangle
    //return a x b
    return a.X()*b.Y() - a.Y()*b.X();
}


//Checks whether a pixel coordinate is occupied by a triangle
//If true, returns the barycentric coordinates at the pixel location
[[nodiscard]] std::optional<std::array<float,3>> Opt_InTriangle(const Vec2f& p, const Triangle& t) {

    //Extract 2D screen coordinates of each vertex
    const auto v0 = Vec2f(t.vertices[0].coords.X(),t.vertices[0].coords.Y());
    const auto v1 = Vec2f(t.vertices[1].coords.X(),t.vertices[1].coords.Y());
    const auto v2 = Vec2f(t.vertices[2].coords.X(),t.vertices[2].coords.Y());

    auto l0{EdgeFunction(v1, v2, p)}; // signed area of the triangle v1v2p multiplied by 2
    auto l1{EdgeFunction(v2, v0, p)}; // signed area of the triangle v2v0p multiplied by 2
    auto l2{EdgeFunction(v0, v1, p)}; // signed area of the triangle v0v1p multiplied by 2

    // Check if the pixel lies inside the triangle
    //In the CCW winding case, this occurs is all signs are -ve
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

    //Get bounding values
    const int min_x = std::min({triangle.vertices[0].coords.X(),triangle.vertices[1].coords.X(),triangle.vertices[2].coords.X()});
    const int max_x = std::max({triangle.vertices[0].coords.X(),triangle.vertices[1].coords.X(),triangle.vertices[2].coords.X()});

    const int min_y = std::min({triangle.vertices[0].coords.Y(),triangle.vertices[1].coords.Y(),triangle.vertices[2].coords.Y()});
    const int max_y = std::max({triangle.vertices[0].coords.Y(),triangle.vertices[1].coords.Y(),triangle.vertices[2].coords.Y()});

    for(int y = min_y; y <= max_y; ++y ) {
        for(int x = min_x;  x <= max_x; ++x) {
            // Check if the pixel lies inside the triangle
            if (auto bary_coords = Opt_InTriangle(Point2f(x,y),triangle); bary_coords) {
                const auto& [l0,l1,l2] = bary_coords.value(); //unpack barycentric coordinates
                const auto& [v0,v1,v2] = triangle.vertices; //Unpack vertices of triangle

                //Interpolate all per-vertex attributes
                const float frag_depth = l0*v0.coords.Z() + l1*v1.coords.Z() + l2*v2.coords.Z(); //Required

                Fragment frag;
                frag.window_coords = Vec3f(x,y,frag_depth);
                frag.tex_coords = l0*v0.tex_coords + l1*v1.tex_coords + l2*v2.tex_coords;
                frag.normal = l0*v0.world_norm + l1*v1.world_norm + l2*v2.world_norm;         

                triangle_frags.push_back(frag);
            }
        }
    }
    return triangle_frags;
}
