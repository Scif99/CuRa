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
#include "light.h"

//Contains all the per-vertex attributes
struct ScreenVertex {
    Vec3f screen_coords; //contains the 2D screen coordinates, as well as the depth
    Vec2f tex_coords;
    Vec3f normal_coords;
    Color3f diffuse;
};

//Note that the vertices need to be specified in CCW order
struct Triangle {
    std::array<ScreenVertex, 3> vertices;
};


//Checks whether a pixel coordinate is occupied by a triangle
//If true, returns the barycentric coordinates at the pixel location
[[nodiscard]] std::optional<std::array<float,3>> Opt_InTriangle(const Vec2f& p, const Triangle& t) {

    //Extract 2D screen coordinates of each vertex
    const auto& v0 = Vec2f(t.vertices[0].screen_coords.X(),t.vertices[0].screen_coords.Y());
    const auto& v1 = Vec2f(t.vertices[1].screen_coords.X(),t.vertices[1].screen_coords.Y());
    const auto& v2 = Vec2f(t.vertices[2].screen_coords.X(),t.vertices[2].screen_coords.Y());

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


//Determines visibility of a triangle and colors it using interpolation
void RasteriseAndColor(const Triangle& triangle, Buffer<Color3f>& image_buf, Buffer<float>& depth_buf, const Buffer<Color3f>& texture_map ) {
    for(int y =0; y < image_buf.Height(); ++y ) {
        for(int x = 0; x <image_buf.Width(); ++x) {
            // Check if the pixel lies inside the triangle
            if (auto bary_coords = Opt_InTriangle(Point2f(x,y),triangle); bary_coords) {
                const auto& [l0,l1,l2] = bary_coords.value(); //unpack barycentric coordinates
                const auto& [v0,v1,v2] = triangle.vertices; //Unpack vertices of triangle

                //Get texture coords via interpolation
                const Vec2f tex = l0*v0.tex_coords + l1*v1.tex_coords + l2*v2.tex_coords;
             
                //USE TEXTURE COORDS TO GET COLOR
                auto fragment_color{texture_map.Get(tex.U(),tex.V())};

                //Lighting
                //interpolate over colors
                const auto interpolated_diffuse = l0*v0.diffuse + l1*v1.diffuse + l2*v2.diffuse;
                fragment_color=fragment_color*interpolated_diffuse;

                //Use Z buffer for hidden surface removal
                const auto z = float{l0*v0.screen_coords.Z() + l1*v1.screen_coords.Z() + l2*v2.screen_coords.Z()};
                if(z>depth_buf.Get(x,y)) {
                    depth_buf.Set(x,y,z);
                    image_buf.Set(x,y,fragment_color);
                }
            }
        }
    }
}

int main() {

	constexpr int height{800};
	constexpr int width{800};

    Buffer<Color3f> image_buffer(height, width);
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
    const Norm3f light_dir{Vec3f{0.f,0.f,-5.f}};
    const DistantLight light = {light_dir, Color3f{1.f,1.f,1.f}};

    //Define the MVP matrices for the model
    auto model_matrix = Mat4f::Identity();
    model_matrix = Translate(model_matrix, Vec3f{0.f,0.f,-1.2f});
    Mat4f view_matrix = LookAt(camera.eye, camera.center, camera.up);
    Mat4f proj_matrix = Projection(-0.5f,-2.5f,-1.f,1.f,-1.f,1.f);

    //Iterate over each face (triangle) in the model
    for(const auto& [vert_indices, tex_indices, norm_indices] : head.Faces()) {

        //We need the world coordinates for lighting
        std::array<Vec4f,3> world;
        for(int i =0;i<3;++i) {world[i] = model_matrix* Vec4f(head.Vertices()[vert_indices[i]],1.f); }

        //Pass vertices through transforms
        std::array<ScreenVertex,3> vertices;
        bool discard = false;
        for(int i =0; i<3; ++i) {
            //VERTEX POSITIONS
            //TRANSFORM TO CLIP SPACE VIA MVP
            auto v_clip = proj_matrix*view_matrix*world[i]; 
            //CLIP
            //if(!v) continue;
            //TRANSFORM TO NDC SPACE VIA PERSPECTIVE DIVIDE 
            const auto p_divide{1.f/v_clip.W()};
            const auto v_ndc = Cartesian(v_clip*p_divide);
            //TRANSFORM TO SCREEN SPACE VIA VIEWPORT
            const auto v_screen = ViewPort(v_ndc,image_buffer.Height(),image_buffer.Width(), true);
            vertices[i].screen_coords = std::move(v_screen);

            //TEXTURE COORDS
            const auto tex_coords = head.TexCoords()[tex_indices[i]]; //Get from model
            const auto tw{texture_map.Width()};
            const auto th{texture_map.Height()}; 
            const auto uv = Vec2f{tex_coords.U()*tw, th-tex_coords.V()*th}; //Scale (flipping in v direction)
            vertices[i].tex_coords = std::move(uv);

            //NORMAL
            const auto normal_transform = Invert(Transpose(model_matrix)).value();//Note the normal transforms differently to point vectors
            const auto local_norm = head.Normals()[norm_indices[i]];
            //Transform to world space
            const auto world_norm = normal_transform*Vec4f(local_norm,1.f);
            vertices[i].normal_coords = UnitVector(Cartesian(world_norm));

            //PER-VERTEX SHADING
            
            //GOURAD SHADING
            auto col = Color3f{0.f,0.f,0.f};
            const float dot = Dot(UnitVector(vertices[i].normal_coords) ,-light.Direction);
            vertices[i].diffuse = dot*light.Color;

            //FLAT SHADING
            //Norm3f triangle_norm{Cross<float,3>(Cartesian(world[1])-Cartesian(world[0]),Cartesian(world[2])-Cartesian(world[0]))};
            //const float dot = Dot(triangle_norm ,-light.Direction); 
            //vertices[i].diffuse = dot*light.Color;

            //Backface culling
            //If the dot product is negative then it means the triangle is not facing the camera (same as light in this case)
            if(dot<0.f) {
                discard = true;
                break;
            }
        }
        if(discard) continue; 

        const Triangle triangle{vertices[0],vertices[1],vertices[2]};

        RasteriseAndColor(triangle, image_buffer, depth_buffer, texture_map);
    }

    //Create image from buffer and write to file
    PPMImage image{std::move(image_buffer)};
    std::ofstream out_file("image.ppm");
	if(!out_file) {std::cerr<<"Error creating file\n"; return 1;};
	image.Write(out_file);
}
