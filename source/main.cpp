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
#include "gourad_shader.h"
#include "line.h"
#include "math.h"
#include "model.h"
#include "ppm_image.h"
#include "vec.h"
#include "mat.h"
#include "pipeline.h"
#include "light.h"
#include "shader.h"

//Note that the vertices need to be specified in CCW order
struct Triangle {
    std::array<ProcessedVertex, 3> vertices;
};

//std::optional<Triangle> Clipped(const Triangle& triangle) {};

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


template<typename T>
T BarycentricInterpolate() {

}

//Determines visibility of a triangle
//In terms of the pipeline, rakes a triangle primitive and breaks it down into fragments
//Interpolates any per-vertex attributes to get a value for the fragment
std::vector<Fragment> Rasterise(const Triangle& triangle, const Buffer<Color3f>& image_buf) {

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

int main() {

	constexpr int height{800};
	constexpr int width{800};

    Buffer<Color3f> image_buffer(height, width);
    Buffer<float> depth_buffer(height, width, -std::numeric_limits<float>::max()); //Create Depth buffer Assume +z is towards camera?

    //Load model
    const Model head("assets/models/head.obj");
    //Load texture for model
    const Buffer<Color3f> diffuse_map = ParsePPMTexture("assets/textures/head_diffuse.ppm");
    const Buffer<Color3f> specular_map = ParsePPMTexture("assets/textures/head_spec.ppm");


    //Initialise scene entities
    const Camera camera(Vec3f{1.f,1.f,3.f}, //eye
                        Vec3f{0.f,0.f,0.f}, //center
                        Vec3f{0.f,1.f,0.f}); //up
    
    //Directional (distant) light
    const Norm3f light_dir{Vec3f{0.f,0.f,-1.f}};
    const DistantLight light = {light_dir, 
                                Color3f{0.2f,0.2f,0.2f}, //diffuse
                                Color3f{0.5f,0.5f,0.5f}, //diffuse
                                Color3f{1.f,1.f,1.f} }; //specular

    auto model_matrix = Mat4f::Identity(); //Note it is defined outside the loop, as it applies to ALL vertices in the model
    //model_matrix = Translate(model_matrix, Vec3f{0.f,0.f,-1.2f});
    Mat4f view_matrix = LookAt(camera.eye, camera.center, camera.up);
    Mat4f proj_matrix = Projection(-2.f,-4.f,-1.f,1.f,-1.f,1.f);

    //Create Shader
    GouradShader g_shader;
    //Set Uniforms in shader
    g_shader.SetUniform("model", model_matrix);
    g_shader.SetUniform("view", view_matrix);
    g_shader.SetUniform("projection", proj_matrix);
    
    g_shader.SetUniform("light_dir", light.Direction);
    g_shader.SetUniform("light_ambient", light.Ambient);
    g_shader.SetUniform("light_diffuse", light.Diffuse);
    g_shader.SetUniform("light_specular", light.Specular);
    g_shader.SetUniform("view_pos", camera.eye);


    //Set textures
    g_shader.SetTexture("diffuse", &diffuse_map);
    g_shader.SetTexture("specular", &specular_map);


    //Iterate over each face (triangle) in the model
    for(const auto& [vert_indices, tex_indices, norm_indices] : head.Faces()) {


        Triangle triangle;

        //Pass vertices through transforms
        for(int i =0; i<3; ++i) {

            //Extract vertex attributes from the model using face indices
            const auto v = head.Vertices()[vert_indices[i]]; 
            const auto n = UnitVector(head.Normals()[norm_indices[i]]);
            const auto tex_coords = head.TexCoords()[tex_indices[i]];

            VertexAttributes vert  = {v,n,tex_coords};
            //Run Vertex Shader 
            const auto processed = g_shader.PerVertex(vert);

            //PRIMITIVE ASSEMBLY
            triangle.vertices[i] = processed;

        }

        //POST_PROCESSING
        

        //Clipping
        //std::optional<Triangle> clipped = Clipped(triangle);
        //if(!clipped) continue;
        //triangle = clipped.value();

        //Convert to Window Space
        std::for_each(triangle.vertices.begin(),triangle.vertices.end(), [&](ProcessedVertex& vertex) {
                const auto p_divide{1.f/vertex.clip_coords.W()};
                const auto v_ndc = Cartesian(vertex.clip_coords*p_divide);//Apply perspective divide
                const auto v_screen = ViewPort(v_ndc,image_buffer.Height(),image_buffer.Width(), true); //Apply viewport transform
                vertex.clip_coords = Vec4f(v_screen,1.f);

                //Also need to scale texture coordinates to the texture's dimensions
                const auto tw{diffuse_map.Width()};
                const auto th{diffuse_map.Height()}; 
                vertex.tex_coords = Vec2f(vertex.tex_coords.U()*tw, th - vertex.tex_coords.V()*th);
        });


        //Culling
        //Backface culling
        //If the dot product is negative then it means the triangle is not facing the camera (same as light in this case)
        // auto angle = Dot()
        // if(dot<0.f) continue;


        //Rasterise the triangle into fragments
        const std::vector<Fragment> vec_frags = Rasterise(triangle, image_buffer);
        
        //FRAGMENT PROCESSING
        for(const auto& fragment : vec_frags) {
            //Run fragment shader
            const auto& [pos,col] = g_shader.PerFragment(fragment); 

            //Depth Test
            //Remember that smaller z means further away (camera faces -z)
            if(pos.Z()>depth_buffer.Get(pos.X(),pos.Y())) {
                depth_buffer.Set(pos.X(),pos.Y(),pos.Z());
                image_buffer.Set(pos.X(),pos.Y(),col);
            }
        }
    }

    //Create image from buffer and write to file
    PPMImage image{std::move(image_buffer)};
    std::ofstream out_file("image.ppm");
	if(!out_file) {std::cerr<<"Error creating file\n"; return 1;};
	image.Write(out_file);
}
