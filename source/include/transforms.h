#ifndef TRANSFORM_H
#define TRANSFORM_H

#include <array>
#include <cmath>
#include <optional>

#include "vec.h"
#include "mat.h"

//Contains the important transforms present in the graphics pipeline

/// @brief Builds a view matrix.
/// @brief Technically constructs a matrix that transforms vectors in the standard basis to vectors in the camera basis.
/// @brief By convention, the camera will be facing in the -z direction
/// @param eye Position of the camera
/// @param center Direction in which the camera is facing
/// @param up Defines the orientation of the camera
/// @return View matrix
[[nodiscard]] Mat4f LookAt(const Vec3f& eye, const Vec3f& center, const Vec3f& up) {
    //Construct an orthonormal basis
    const auto w = Norm3f{eye - center}; //'forward' axis. By convention, the camera should be facing in the -z direction
    const auto u = Norm3f{Cross<float,3>(up,w)}; //'right' axis
    const auto v = Norm3f{Cross<float, 3>(w,u)}; //'up' axis
        
    //The goal of the view matrix is to transform from our standard basis to the camera basis
    //NOTE: Don't forget the translation.
    //Therefore the view matrix is just the inverse of the transformation matrix from the standard basis to our camera basis.
    //To avoid inverting any matrices, note that the inverse is equivalent to composing the inverse translation with the inverse transformation matrix, which is orthogonal.
    
    //Define the inverse transformation matrix
    //It is the transpose of the transformation matrix
    Mat4f transform({
        u[0], u[1], u[2], 0.f,
        v[0], v[1], v[2], 0.f,
        w[0], w[1], w[2], 0.f,
        0.f,  0.f,  0.f,  1.f
    });

    //The inverse of the matrix that translates the standard origin to the camera origin
    //This is equivalent to a translation that reverses this movement
    Mat4f translate({
    1.f,  0.f,  0.f,  -eye[0],
    0.f,  1.f,  0.f,  -eye[1],
    0.f,  0.f,  1.f,  -eye[2],
    0.f,  0.f,  0.f,  1.f
    });

    //The view matrix is the composition of these matrices
    return transform*translate;
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
[[nodiscard]] Mat4f Projection(float n, float f, float l, float r, float b, float t) { 

    assert(l<r && b<t && n > f);
    const std::array<float, 16> data = {
        2.f*n/(r-l), 0.f,         (l+r)/(l-r),  0.f,
        0.f,         2.f*n/(t-b), (b+t)/(b-t),  0.f,
        0.f,         0.f,         (f+n)/(n-f),  2.f* f *n/(f-n),
        0.f,         0.f,         1.f,         0.f
    };
    const Mat4f projection_mat(std::move(data));
    return projection_mat;
}


/// @brief Applies the viewport transform 
/// @param ndc_coords Coordinates inside the unit cube [-1,1]
/// @param im_height 
/// @param im_width 
/// @param flip_y 
/// @return 
[[nodiscard]] Vec3f ViewPort(const Vec3f& ndc_coords, int im_height, int im_width, bool flip_y) {

    const auto y_scale{flip_y ? -1.f : 1.f};
    const auto x_pixel = (ndc_coords.X()+1)*im_width/2.f;
    const auto y_pixel = (y_scale*ndc_coords.Y()+1)*im_height/2.f;
    return Vec3f(x_pixel,y_pixel, ndc_coords.Z());
}
#endif