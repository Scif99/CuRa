
#include <algorithm>
#include <cmath>
#include <chrono>
#include <iostream>
#include <memory>
#include <numeric>
#include <optional>
#include <span>
#include <vector>

#include "cura/buffer.h"
#include "cura/camera.h"
#include "cura/depth_shader.h"
#include "cura/light.h"
#include "cura/line.h"
#include "cura/mat.h"
#include "cura/math.h"
#include "cura/model.h"
#include "cura/normal_map_shader.h"
#include "cura/pipeline.h"
#include "cura/rasterizer.h"
#include "cura/shader.h"
#include "cura/texture.h"
#include "cura/transforms.h"
#include "cura/triangle.h"
#include "cura/vec.h"

std::optional<Triangle> Clipped(const Triangle& triangle) {return triangle;}

struct Statistics {
    float num_triangles; //Total number of input triangles
    float culled_triangles;
    float clipped_triangles{0};
    std::chrono::duration<double> time;

    void Log(std::ostream& os) {
        std::cout<<"Total Model triangles:\t"<< num_triangles<<"\n";
        std::cout<<"Culled triangles:\t" <<culled_triangles<<"\n";
        std::cout<<"time Taken:\t" <<time.count()<<"s\n";
    }
};

int main() {

    Statistics stats;
    const auto start = std::chrono::steady_clock::now();

	constexpr int height{800};
	constexpr int width{800};

    Buffer<Color3f> image_buffer(height, width);
    Buffer<float> depth_buffer(height, width, -std::numeric_limits<float>::max()); //Create Depth buffer Assume +z is towards camera?
    //Buffer<float> shadow_map(height, width, -std::numeric_limits<float>::max()); //For shadows


    //Load model
    const Model head("assets/models/head.obj");
    const Buffer<Color3f> diffuse_map =  ParsePPMTexture("assets/textures/head_diffuse.ppm");
    const Buffer<Color3f> specular_map = ParsePPMTexture("assets/textures/head_spec.ppm");
    const Buffer<Vec3f> normal_map = ParsePPMTexture("assets/textures/head_nm.ppm");

    // const Model head("assets/models/diablo3_pose.obj");
    // const Buffer<Color3f> diffuse_map =  ParsePPMTexture("assets/textures/diablo3_pose_diffuse.ppm");
    // const Buffer<Color3f> specular_map = ParsePPMTexture("assets/textures/diablo3_pose_spec.ppm");
    // const Buffer<Vec3f> normal_map = ParsePPMTexture("assets/textures/diablo3_pose_nm.ppm");



    //Initialise scene entities
    const Camera camera(Vec3f{0.f,0.f,1.5f}, //eye
                        Vec3f{0.f,0.f,0.f}, //center
                        Vec3f{0.f,1.f,0.f}); //up
    
    //Directional (distant) light
    const Norm3f light_dir{Vec3f{0.f,0.f,-1.f}};
    const DistantLight light = {light_dir, 
                                Color3f{0.2f,0.2f,0.2f}, //ambient
                                Color3f{0.7f,0.7f,0.7f}, //diffuse
                                Color3f{1.f,1.f,1.f} }; //specular

    //Note the transformations are defined outside the loop, as they apply to ALL vertices in the model
    auto model_matrix = Mat4f::Identity(); 
    //model_matrix = Translate(model_matrix, Vec3f{0.f,0.f,-1.2f});
    Mat4f view_matrix = LookAt(camera.eye, camera.center, camera.up);
    Mat4f proj_matrix = Projection(-1.f,-4.f,-1.f,1.f,-1.f,1.f);

    //Create Shader
    NormalMapShader nm_shader;
    //Set Uniforms in shader
    nm_shader.SetUniform("model", model_matrix);
    nm_shader.SetUniform("view", view_matrix);
    nm_shader.SetUniform("projection", proj_matrix);
    
    nm_shader.SetUniform("light_dir", light.Direction);
    nm_shader.SetUniform("light_ambient", light.Ambient);
    nm_shader.SetUniform("light_diffuse", light.Diffuse);
    nm_shader.SetUniform("light_specular", light.Specular);
    nm_shader.SetUniform("view_pos", camera.eye);

    //Set textures
    nm_shader.SetTexture("diffuse", &diffuse_map);
    nm_shader.SetTexture("specular", &specular_map);
    nm_shader.SetTexture("normal", &normal_map);

    //Iterate over each face (triangle) in the model
    for(const auto& [vert_indices, norm_indices, tex_indices] : head.Faces()) {
        ++stats.num_triangles;

        Triangle triangle;
        for(int i =0; i<3; ++i) {

            //Extract vertex attributes from the model using face indices
            const auto v = head.Vertices()[vert_indices[i]]; 
            const auto n = UnitVector(head.Normals()[norm_indices[i]]);
            const auto tex_coords = head.TexCoords()[tex_indices[i]];

           //Run Vertex Shader 
            Vertex vert  = {v,n,tex_coords};
            const auto processed = nm_shader.PerVertex(vert);

            //Assemble Primitives
            triangle.vertices[i] = processed;

        }

        //POST_PROCESSING
        

        //Clipping
        //If all 3 vertices lie outside frustum, then we clip the whole thing
        //Partially clipped triangles will still be drawn...
        std::optional<Triangle> clipped_triangle = Clipped(triangle);
        if(!clipped_triangle) {
            continue;
        }
        triangle = clipped_triangle.value();

        //Transform to Window Space
        std::for_each(triangle.vertices.begin(),triangle.vertices.end(), [&](ShadedVertex& vertex) {
                const auto p_divide{1.f/vertex.coords.W()}; 
                const auto v_ndc = Cartesian(vertex.coords*p_divide);//Apply perspective divide
                const auto v_screen = ViewPort(v_ndc,image_buffer.Height(),image_buffer.Width(), true); //Apply viewport transform
                vertex.coords = Vec4f(v_screen,1.f); //store window space coords
        });

        //Backface culling
        //If the triangle is not facing the view direction, we don't have to render it
        const auto face_norm = Norm3f(Cross(Cartesian(triangle.vertices[1].coords - triangle.vertices[0].coords), 
                                            Cartesian(triangle.vertices[2].coords - triangle.vertices[0].coords)));
        if(face_norm.Z()>0) {
           ++stats.culled_triangles;
           continue;
        }


        //Rasterise the triangle into fragments
        const auto fragments = Rasterise(triangle, image_buffer);
        
        //FRAGMENT PROCESSING
        for(const auto& frag : fragments) {
            //Run fragment shader
            const auto& [pos,col] = nm_shader.PerFragment(frag); 

            //Depth Test
            //Remember that smaller z means further away (camera faces -z)
            if(pos.Z()>depth_buffer.Get(pos.X(),pos.Y())) {
                depth_buffer.Set(pos.X(),pos.Y(),pos.Z());
                image_buffer.Set(pos.X(),pos.Y(),col);
            }

            //Blending
        }
    }

    //Create image from buffer and write to file
    std::ofstream out_file("image.ppm");
	if(!out_file) {std::cerr<<"Error creating file\n"; return 1;};
	image_buffer.Write(out_file);

    const auto end = std::chrono::steady_clock::now();
    stats.time = end - start;
    stats.Log(std::cout);

}
