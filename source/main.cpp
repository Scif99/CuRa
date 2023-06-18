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
#include "mat.h"


/*
OVERVIEW
//Initialise image, camera etc...
for(const auto& entity: Scene) {
    for(const auto& triangle : entity) {
        auto Projected = Project(triangle)
        Rasterise(triangle, image);
        Shade(triangle)
    }
}
*/

[[nodiscard]] float InterpolateDepth(std::span<const float,3> bary, std::span<const float, 3> v_depth) {
    return std::inner_product(bary.begin(), bary.end(), v_depth.begin(), 0.f);
};

//cam_dist is the z-distance of the camera from the origin
[[nodiscard]]  Mat4f CreateProjMat(const Vec3f& eye, const Vec3f& center) {
    auto p = Mat4f::Identity();
    p(3,2) = -1.f/(eye-center).Length();
    return p;
}



//A vertex consists of a geometric position and a color
//Note the coordinates are in 2D screen space (pixels) with the 3rd coordinate being the depth 
struct ScreenVertex {
    Vec2f pixel_coords;
    Vec2f tex_coords;
    float depth_;
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

                //For all per-vertex attributes, we can now interpolate using barycentric coords (just color for now)
                //For example, the red component at pixel p is the weighted sum of the red components of t, weighted by barycentric coords
                //Note that v0.col is the RGB color of the 1st vertex of the triangle 
                //THIS IS CALLED GOURAD INTERPOLATION
                // const auto r = l0*v0.col.R() + l1*v1.col.R() + l2*v2.col.R();
                // const auto g = l0*v0.col.G() + l1*v1.col.G() + l2*v2.col.G();
                // const auto b = l0*v0.col.B() + l1*v1.col.B() + l2*v2.col.B();
                // auto col = Color3{r,g,b};

                //Get texture coords via interpolation
                const auto tex_u{l0*v0.tex_coords[0] + l1*v1.tex_coords[0] + l2*v2.tex_coords[0]};
                const auto tex_v{l0*v0.tex_coords[1] + l1*v1.tex_coords[1] + l2*v2.tex_coords[1]};
                
                //USE TEXTURE COORDS TO GET COLOR
                auto col{texture_map.Get(tex_u,tex_v)};

                //Lighting                                                       
                col*=triangle.intensity[0]; 

                //Use Z buffer for hidden surface removal
                const auto z{InterpolateDepth(bary_coords.value(), std::array{v0.depth_,v1.depth_,v2.depth_})};
                if(z>depth_buf.Get(x,y)) {
                    depth_buf.Set(x,y,z);
                    image_buf.Set(x,y,col);
                }
            }
        }
    }
}


/// @brief Builds a view matrix.In other words, Defines the camera
/// @brief Technically constructs a matrix that transforms vectors in the standard basis to vectors in the camera basis.
/// @brief By convention, the camera will be facing in the -z direction
/// @param eye Position of the camera
/// @param center Direction in which the camera is facing
/// @param up Defines the orientation of the camera
/// @return View matrix
Mat4f LookAt(const Vec3f& eye, const Vec3f& center, const Vec3f& up) {
    //Construct an orthonormal basis
    const auto z = Norm3f{eye - center}; //'forward' axis. By convention, the camera should be facing in the -z direction
    const auto x = Norm3f{Cross<float,3>(up,z)}; //'left' axis
    const auto y = Norm3f{Cross<float, 3>(z,x)}; //'up' axis
        
    //The goal of the view matrix is to transform from our standard basis to the camera basis.
    //Therefore the view matrix is just the inverse of the transformation matrix from the standard basis to our camera basis.
    //Since the camera basis is orthogonal, the inverse is equivalent to the transpose
    Mat4f view_matrix;
    for (int i=0; i<3; i++) {
    view_matrix(0,i) = x[i];
    view_matrix(1,i) = y[i];
    view_matrix(2,i) = z[i];
    view_matrix(i,3) = -eye[i];
    }
    return view_matrix;
}


/// @brief Projects a point 
/// @brief Note the z coordinate of the original point is kept as the depth
/// @param coords Coordinates of the point to be projected (should be in camera space) 
/// @param eye 
/// @param center 
/// @param im_height 
/// @param im_width 
/// @param flip_y 
/// @return 3D vectors containing the coordinates of the projected point, along with the depth 
Vec3f ProjectAndViewPort(const Vec4f& coords, int im_height, int im_width, bool flip_y) { 

    //Define projection matrix
    auto projection_mat = Mat4f::Identity();
    projection_mat(3,2) = -1.f;
    projection_mat(3,3) = 0.f;

    //Apply the projection matrix
    const auto h_projected_coords{projection_mat*coords};

    //Convert back to 3d, applying perspective divide
    const auto p_divide{1.f/h_projected_coords.W()};
    Vec3f projected_coord(h_projected_coords.X()*p_divide, 
                          h_projected_coords.Y()* p_divide,
                          h_projected_coords.Z()*p_divide);

    const auto y_scale{flip_y ? -1.f : 1.f};
    projected_coord[0] = (projected_coord.X()+1)*im_width/2.f;
    projected_coord[1] = (y_scale*projected_coord.Y()+1)*im_height/2.f;

    //Note we keep the original z coord as the depth.
    //Consider two vertices that lie on the same projective line.
    //They will both be projected to the same point, 
    const auto depth{coords.Z()};
    return Vec3f{projected_coord.X(),projected_coord.Y(), depth};
}

struct Camera{
    Vec3f eye; //Position of the camera
    Vec3f center; //Direction the camera is looking
    Norm3f up; //Defines orientation of the camera 
};



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
    for(const auto& [vert_indices, tex_indices] : head.Faces()) {


        //Get the coordinates in local space     
        const auto w0{head.Vertices()[vert_indices[0]]};
        const auto w1{head.Vertices()[vert_indices[1]]};
        const auto w2{head.Vertices()[vert_indices[2]]};

        //Convert to homogeneous coords.
        auto homo0 = Vec4f{w0,1.f};
        auto homo1 = Vec4f{w1,1.f};
        auto homo2 = Vec4f{w2,1.f};

        //Convert to world space
        //Use SORT for order of operations (Scale -> Rotate -> Translate)
        auto model_matrix = Mat4f::Identity();
        model_matrix = Translate(model_matrix, Vec3f{0.f,0.f,-2.f});
        homo0 = model_matrix*homo0;
        homo1 = model_matrix*homo1;
        homo2 = model_matrix*homo2;


        //Convert to camera space
        const auto eye = Vec3f{0.f,0.f,1.f};
        const auto center = Vec3f{0.f,0.f,-1.f};
        const auto up = Vec3f{0.f,1.f,0.f};
        Mat4f view_matrix = LookAt(eye, center, up);
        homo0 = view_matrix*homo0;
        homo1 = view_matrix*homo1;
        homo2 = view_matrix*homo2;


        //Apply projection and viewport to get screen (pixel) coordinates

        //TODO what about transform to ndc?
        const auto s0{ProjectAndViewPort(homo0,image_buffer.Height(),image_buffer.Width(),true)};
        const auto s1{ProjectAndViewPort(homo1,image_buffer.Height(),image_buffer.Width(),true)};
        const auto s2{ProjectAndViewPort(homo2,image_buffer.Height(),image_buffer.Width(),true)};

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
        Norm3f norm{Cross<float,3>(w1-w0,w2-w0)};
        const float dot = Dot(norm,-light_dir); //-light_dir is the vector from surface to light source
        //Backface culling
        //If the dot product is negative then it means the triangle is not facing the camera (same as light in this case)
        if(dot>0.f) {
            col = Color3f{dot,dot,dot};
        }

        //Store the attributes in a Vertex object
        const ScreenVertex a{Vec2f{s0.X(),s0.Y()}, uv0, w0.Z()};
        const ScreenVertex b{Vec2f{s1.X(),s1.Y()}, uv1, w1.Z()};
        const ScreenVertex c{Vec2f{s2.X(),s2.Y()}, uv2, w2.Z()};

        const Triangle triangle{a,b,c, col};
        RasteriseAndColor(triangle, image_buffer, depth_buffer, texture_map);
    }

    //Create image from buffer and write to file
    PPMImage image{std::move(image_buffer)};
    std::ofstream out_file("image.ppm");
	if(!out_file) {std::cerr<<"Error creating file\n"; return 1;};
	image.Write(out_file);
}
