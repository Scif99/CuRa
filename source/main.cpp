#include <cmath>
#include <iostream>
#include <memory>
#include <optional>
#include <span>
#include <vector>
#include <numeric>
#include <algorithm>

#include "buffer.h"
#include "camera.h"
#include "line.h"
#include "math.h"
#include "model.h"
#include "ppm_image.h"
#include "vec.h"
#include "mat.h"
#include "pipeline.h"

//A vertex consists of a geometric position and a color
//Note the coordinates are in 2D screen space (pixels) with the 3rd coordinate being the depth 
struct ScreenVertex {
    Vec2f pixel_coords;
    Vec2f tex_coords;
    float depth;
};

//Note that the vertices need to be specified in CCW order
struct Triangle {
    std::array<ScreenVertex, 3> vertices;
    Color3f intensity;
};


//Checks whether a pixel coordinate is occupied by a triangle
//If true, returns the barycentric coordinates at the pixel location
[[nodiscard]] std::optional<std::array<float,3>> Opt_InTriangle(const Vec2f& p, const Triangle& t) {
    const auto& [v0,v1,v2] = t.vertices; //Unpack vertices of triangle
    auto l0{EdgeFunction(v1.pixel_coords, v2.pixel_coords, p)}; // signed area of the triangle v1v2p multiplied by 2
    auto l1{EdgeFunction(v2.pixel_coords, v0.pixel_coords, p)}; // signed area of the triangle v2v0p multiplied by 2
    auto l2{EdgeFunction(v0.pixel_coords, v1.pixel_coords, p)}; // signed area of the triangle v0v1p multiplied by 2

    // Check if the pixel lies inside the triangle
    if (l0 >= 0 && l1 >= 0 && l2 >= 0) {
        const auto area = float{EdgeFunction(v0.pixel_coords, v1.pixel_coords, v2.pixel_coords)}; // The edge function with a triangles vertices as its arguments results in twice the area of triangle
        // Compute barycentric coordinates
        l0 /= area;
        l1 /= area;
        l2 /= area;
        return std::array{l0,l1,l2};
    }
    return std::nullopt;
}


//Determines visibility of a triangle and colors it using interpolation
void RasteriseAndColor(const Triangle& triangle, Buffer<Color3f>& image_buf, Buffer<float>& depth_buf, const Buffer<Color3f>& texture_map ) {
    for(int y =0; y < image_buf.Height(); ++y ) {
        for(int x = 0; x <image_buf.Width(); ++x) {
            // Check if the pixel lies inside the triangle
            if (auto bary_coords = Opt_InTriangle(Point2f(x,y),triangle); bary_coords) {
                const auto& [l0,l1,l2] = bary_coords.value(); //unpack barycentric coordinates
                const auto& [v0,v1,v2] = triangle.vertices; //Unpack vertices of triangle

                //Get texture coords via interpolation
                const auto tex_u{l0*v0.tex_coords[0] + l1*v1.tex_coords[0] + l2*v2.tex_coords[0]};
                const auto tex_v{l0*v0.tex_coords[1] + l1*v1.tex_coords[1] + l2*v2.tex_coords[1]};
                
                //USE TEXTURE COORDS TO GET COLOR
                auto col{texture_map.Get(tex_u,tex_v)};

                //Lighting                                                       
                col=col*triangle.intensity; 

                //Use Z buffer for hidden surface removal
                const auto z = float{l0*v0.depth + l1*v1.depth + l2*v2.depth};
                if(z>depth_buf.Get(x,y)) {
                    depth_buf.Set(x,y,z);
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

    //Initialise scene entities
    const Camera camera(Vec3f{0.f,0.f,0.f},
                        Vec3f{0.f,0.f,-1.f},
                        Vec3f{0.f,1.f,0.f});
    
    //Directional (distant) light
    Norm3f light_dir{Vec3f{0.f,0.f,-5.f}};

    //Iterate over each face (triangle) in the model
    for(const auto& [vert_indices, tex_indices] : head.Faces()) {

        auto model_matrix = Mat4f::Identity();
        model_matrix = Translate(model_matrix, Vec3f{0.f,0.f,-1.2f});
        Mat4f view_matrix = LookAt(camera.eye, camera.center, camera.up);
        Mat4f proj_matrix = Projection(-0.5f,-2.5f,-1.f,1.f,-1.f,1.f);

        std::array<Vec4f,3> world;
        for(int i =0;i<3;++i) {world[i] = model_matrix* Vec4f(head.Vertices()[vert_indices[i]],1.f); }

        std::array<Vec3f, 3> vertices;
        for(int i =0; i<3; ++i) {
            auto v = proj_matrix*view_matrix*world[i]; 
            //if(!v) continue;
            const auto p_divide{1.f/v.W()};

            auto v_clip = Cartesian(v*p_divide);
            v_clip = ViewPort(v_clip,image_buffer.Height(),image_buffer.Width(), true);
            vertices[i] = std::move(v_clip);
        }

        //Get the texture coordinates
        const auto t0 = head.TexCoords()[tex_indices[0]];
        const auto t1 = head.TexCoords()[tex_indices[1]];
        const auto t2 = head.TexCoords()[tex_indices[2]];

        //Scale them to the texture map
        //Note we Need to flip the v axis
        const auto tw{texture_map.Width()};
        const auto th{texture_map.Height()}; 
        const auto uv0 = Vec2f{t0.U()*tw, th-t0.V()*th};
        const auto uv1 = Vec2f{t1.U()*tw, th-t1.V()*th};
        const auto uv2 = Vec2f{t2.U()*tw, th-t2.V()*th};

        //Do some basic per-triangle shading
        //This results in whats called 'flat shading'
        auto col = Color3f{0.f,0.f,0.f};
        Norm3f norm{Cross<float,3>(Cartesian(world[1])-Cartesian(world[0]),Cartesian(world[2])-Cartesian(world[0]))};
        const float dot = Dot(norm,-light_dir); //-light_dir is the vector from surface to light source
        //Backface culling
        //If the dot product is negative then it means the triangle is not facing the camera (same as light in this case)
        if(dot>0.f) {
            col = Color3f{dot,dot,dot};
        }

        //Store the attributes in a Vertex object
        const ScreenVertex a{Vec2f(vertices[0].X(),vertices[0].Y()), uv0, vertices[0].Z()};
        const ScreenVertex b{Vec2f(vertices[1].X(),vertices[1].Y()), uv1, vertices[1].Z()};
        const ScreenVertex c{Vec2f(vertices[2].X(),vertices[2].Y()), uv2, vertices[2].Z()};

        const Triangle triangle{a,b,c, col};
        RasteriseAndColor(triangle, image_buffer, depth_buffer, texture_map);
    }

    

    //Create image from buffer and write to file
    PPMImage image{std::move(image_buffer)};
    std::ofstream out_file("image.ppm");
	if(!out_file) {std::cerr<<"Error creating file\n"; return 1;};
	image.Write(out_file);
}
