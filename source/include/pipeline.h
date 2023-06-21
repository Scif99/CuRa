#ifndef PIPELINE_H
#define PIPELINE_H

#include <array>
#include <cmath>
#include <optional>

#include "vec.h"
#include "mat.h"



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
std::optional<Vec3f> Project(const Vec4f& coords, float n, float f, float l, float r, float b, float t) { 

    assert(l<r && b<t && n > f);
    const std::array<float, 16> data = {
        2.f*n/(r-l), 0.f,         (l+r)/(l-r),  0.f,
        0.f,         2.f*n/(t-b), (b+t)/(b-t),  0.f,
        0.f,         0.f,         (f+n)/(n-f),  2.f* f *n/(f-n),
        0.f,         0.f,         1.f,         0.f
    };
    const Mat4f projection_mat(std::move(data));

    //Apply the projection matrix
    const auto h_projected_coords{projection_mat*coords};

    //Clip in homogeneous coordinates
    // if(h_projected_coords.X() < w || h_projected_coords.X() > -w || 
    //    -h_projected_coords.Y() < w || -h_projected_coords.Y() > -w || 
    //    -h_projected_coords.Z() < w || -h_projected_coords.Z() > -w ) 
    //    return std::nullopt; 

    //Convert back to 3d, applying perspective divide
    const auto p_divide{1.f/h_projected_coords.W()};
    const Vec3f projected_coord(h_projected_coords.X()*p_divide, 
                                h_projected_coords.Y()*p_divide,
                                h_projected_coords.Z()*p_divide);
    return projected_coord;
}


/// @brief Applies the viewport transform 
/// @param ndc_coords 
/// @param im_height 
/// @param im_width 
/// @param flip_y 
/// @return 
Vec3f ViewPort(const Vec3f& ndc_coords, int im_height, int im_width, bool flip_y) {

    const auto y_scale{flip_y ? -1.f : 1.f};
    const auto x_pixel = (ndc_coords.X()+1)*im_width/2.f;
    const auto y_pixel = (y_scale*ndc_coords.Y()+1)*im_height/2.f;
    return Vec3f(x_pixel,y_pixel,ndc_coords.Z());
}

#endif