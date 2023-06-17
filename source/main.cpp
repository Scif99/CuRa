#include <cmath>
#include <iostream>
#include <memory>
#include <optional>
#include <span>
#include <vector>
#include <numeric>

#include "buffer.h"
#include "line.h"
#include "math.h"
#include "model.h"
#include "ppm_image.h"
#include "vec.h"
/*
OVERVIEW
for(const auto& entity: Scene) {
    std::vector<std::array<int,2> PixCoords = Project(entity);
    Rasterise(entity);
    Shade(entity, shading_model);
}
*/


//Generic function that takes some sequence of vertex attributes, interpolates them
//then returns a vector of interpolated values
// template<typename Container, unsigned S>
// std::vector<float> InterpolateVertexAttrib(std::span<const float, 3> bary, Container&& container) {
//     return {};
// }

// std::array<float,2> InterpolateTexCoords(std::span<const float, 3> bary, std::span<const std::array<float,2>> v_tex) {
//     const auto tex_u = std::inner_product(bary.begin(),bary.end(),v_tex.begin()[0],0.f);
//     const auto tex_v = std::inner_product(bary.begin(),bary.end(),v_tex.begin()[1],0.f);
//     return {tex_u,tex_v};

// }

float InterpolateDepth(std::span<const float,3> bary, std::span<const float, 3> v_depth) {
    return std::inner_product(bary.begin(), bary.end(), v_depth.begin(), 0.f);
};

//A vertex consists of a geometric position and a color
//NOTE THE POSITION IS IN SCREEN SPACE FOR NOW
struct Vertex {
    Vec2f pixel_coords;
    Vec2f tex_coords;
    float depth_;
};

//Note that the vertices need to be specified in CCW order
struct Triangle {
    std::array<Vertex, 3> vertices;
    Color3f intensity;
};


//Checks whether a pixel coordinate is occupied by a triangle
//If true, returns the barycentric coordinates at the pixel location
[[nodiscard]] std::optional<std::array<float,3>> Opt_InTriangle(const Vec2f& p, const Triangle& t) {
    const auto& [v0,v1,v2] = t.vertices; //Unpack vertices of triangle
    float w0 = EdgeFunction(v1.pixel_coords, v2.pixel_coords, p); // signed area of the triangle v1v2p multiplied by 2
    float w1 = EdgeFunction(v2.pixel_coords, v0.pixel_coords, p); // signed area of the triangle v2v0p multiplied by 2
    float w2 = EdgeFunction(v0.pixel_coords, v1.pixel_coords, p); // signed area of the triangle v0v1p multiplied by 2

    // Check if the pixel lies inside the triangle
    if (w0 >= 0 && w1 >= 0 && w2 >= 0) {
        float area = EdgeFunction(v0.pixel_coords, v1.pixel_coords, v2.pixel_coords); // The edge function with a triangles vertices as its arguments results in twice the area of triangle
        // Compute barycentric coordinates
        w0 /= area;
        w1 /= area;
        w2 /= area;
        return std::array{w0,w1,w2};
    }
    return std::nullopt;
}


//Processing is something like computing the color, depth
void ProcessFragment() {};

//Determines visibility of a triangle and colors it using interpolation
void RasteriseAndColor(const Triangle& triangle, Buffer<Color3f>& image_buf, Buffer<float>& depth_buf, const Buffer<Color3f>& texture_map ) {
    for(int y =0; y < image_buf.Height(); ++y ) {
        for(int x = 0; x <image_buf.Width(); ++x) {
            auto p = Vec2f(x,y); //TODO move p to centre of pixel?
            // Check if the pixel lies inside the triangle
            if (auto bary_coords = Opt_InTriangle(p,triangle); bary_coords) {
                const auto& [l0,l1,l2] = bary_coords.value(); //unpack barycentric coordinates
                const auto& [v0,v1,v2] = triangle.vertices; //Unpack vertices of triangle

                //For all per-vertex attributes, we can now interpolate using barycentric coords (just color for now)
                //For example, the red component at pixel p is the weighted sum of the red components of t, weighted by barycentric coords
                //Note that v0.col is the RGB color of the 1st vertex of the triangle 
                //THIS IS CALLED GOURAD INTERPOLATION
                // const auto r = l0*v0.col.R() + l1*v1.col.R() + l2*v2.col.R();
                // const auto g = l0*v0.col.G() + l1*v1.col.G() + l2*v2.col.G();
                // const auto b = l0*v0.col.B() + l1*v1.col.B() + l2*v2.col.B();
                // auto col = Color3{r,g,b};

                //Get texture coords via interpolation
                const auto tex_u = l0*v0.tex_coords[0] + l1*v1.tex_coords[0] + l2*v2.tex_coords[0];
                const auto tex_v = l0*v0.tex_coords[1] + l1*v1.tex_coords[1] + l2*v2.tex_coords[1];

                //USE TEXTURE COORDS TO GET COLOR
                auto col = texture_map.Get(tex_u,tex_v);
                col*=triangle.intensity[0]; 

                //GET DEPTH OF PIXEL VIA INTERPOLATION
                const auto z = InterpolateDepth(bary_coords.value(), std::array{v0.depth_,v1.depth_,v2.depth_});
                //IS THIS TRIANGLE THE CLOSEST ONE FOR THIS PIXEL???
                if(z>depth_buf.Get(x,y)) {
                    depth_buf.Set(x,y,z);

                //normally z lies in [-1,1]
                image_buf.Set(x,y,col);
                }
            }
        }
    }
}



int main() {

	constexpr int height{800};
	constexpr int width{800};
    
    Buffer<Color3f> image_buffer(height, width); //Create Depth buffer Assume +z is towards camera?
    Buffer<float> depth_buffer(height, width, -std::numeric_limits<float>::max()); //Create Depth buffer Assume +z is towards camera?

    //Load model
    Model head("assets/models/head.obj");
    //Load texture for model
    Buffer<Color3f> texture_map = ParsePPMTexture("assets/textures/head_diffuse.ppm");

    Norm3f light_dir{Vec3f{0.f,0.f,-5.f}};

    // //Iterate over each face (triangle) in the model
    for(const auto& [v_idx, tex_idx] : head.Faces()) {

        //Get the world coordinates in the unit cube       
        const auto w0 = head.Vertices()[v_idx[0]];
        const auto w1 = head.Vertices()[v_idx[1]];
        const auto w2 = head.Vertices()[v_idx[2]];

        //CONVERT TO SCREEN COORDINATES VIA AN ORTHOGRAPHIC PROJECTION
        //The original positions are in 3D where each coord lies in [-1,1]
        //NOTE we store the z coord (depth) separately
        Vec2f s0{(w0.X()+1)*width/2.f, (-w0.Y()+1)*height/2.f};
        Vec2f s1{(w1.X()+1)*width/2.f, (-w1.Y()+1)*height/2.f};
        Vec2f s2{(w2.X()+1)*width/2.f, (-w2.Y()+1)*height/2.f};

        //Get the texture coordinates
        const auto t0 = head.TexCoords()[tex_idx[0]];
        const auto t1 = head.TexCoords()[tex_idx[1]];
        const auto t2 = head.TexCoords()[tex_idx[2]];

        //Scale them to the texture map
        //Note we Need to flip the v axis
        const auto tw = texture_map.Width();
        const auto th = texture_map.Height(); 
        const auto uv0 = Vec2f{t0[0]*tw,th - t0[1]*th};
        const auto uv1 = Vec2f{t1[0]*tw,th - t1[1]*th};
        const auto uv2 = Vec2f{t2[0]*tw,th - t2[1]*th};

        //Do some basic per-triangle shading
        auto col = Color3f{0.f,0.f,0.f};
        Norm3f norm{Cross<float,3>(w1-w0,w2-w0)};
        const float dot = (norm[0]*(-light_dir[0])) +  (norm[1]*(-light_dir[1])) + (norm[2]*(-light_dir[2])); //-light_dir is the vector from surface to light source
        //Backface culling
        //If the dot product is negative then it means the triangle is not facing the camera (same as light in this case)
        if(dot>0.f) {
            col = Color3f{dot,dot,dot};
        }

        //Store the attributes in a Vertex object
        const Vertex a{s0, uv0, w0.Z()};
        const Vertex b{s1, uv1, w1.Z()};
        const Vertex c{s2, uv2, w2.Z()};

        const Triangle triangle{a,b,c, col};

        RasteriseAndColor(triangle, image_buffer, depth_buffer, texture_map);


    }

    //Create image from buffer and write to file
    PPMImage image{std::move(image_buffer)};
    std::ofstream out_file("image.ppm");
	if(!out_file) {std::cerr<<"Error creating file\n"; return 1;};
	image.Write(out_file);
}
