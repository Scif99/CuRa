#pragma once


#include <cura/math.h>


//Contains the important transforms present in the graphics pipeline

/// @brief Builds a view matrix.
/// @brief This matrix transforms vectors from the standard basis to the camera basis.
/// @brief By convention, the 'forward' direction in camera space is aligned with the -z direction.
/// @param eye Position of the camera in world spaces.
/// @param center Direction in which the camera is facing in world space.
/// @param up Defines the orientation of the camera.
/// @return View matrix
[[nodiscard]] inline Mat44f LookAt(const Vec3f& eye, const Vec3f& center, const Vec3f& up) {
    //Construct an orthonormal basis
    const auto v = la::normalize(eye - center); //'forward' axis. By convention, the camera should be facing in the -z direction
    const auto r = la::normalize(la::cross(up,v)); //'right' axis
    const auto u = la::normalize(la::cross(v,r)); //'up' axis

    //To understand how the matrix is constructed, note that is a composition of the following transforms:
    //Translate everything such that the camera lies at the origin
    //Apply the change-of-basis matrix.
    return Mat44f{
        {r.x,    u.x,    v.x,     0.f},
        {r.y,    u.y,    v.y,     0.f},
        {r.z,    u.z,    v.z,     0.f},
        {-eye.x ,-eye.y ,-eye.z  ,1.f}
    };


}


/// @brief Constructs an orthographic projection matrix
/// @brief Assumes that the viewer oriented in a right-handed coordinate system, looking towards the -z direction, with the y axis pointing up. 
/// @param l x-coordinate of left plane
/// @param r x-coordinate of right plane
/// @param b y-coordinate of bottom plane
/// @param t y-coordinate of top plane
/// @param n z-coordinate of near plane
/// @param f z-coordinate of far plane (<n)
/// @return Matrix representing the associated projection
[[nodiscard]] inline Mat44f OrthographicProjection(float l, float r, float b, float t, float n, float f) {
    
    //Constructing the matrix can be understood by visualising what it does:
    //First it translates the AABB defined by the bounds provided to the origin.
    //Then it scales this AABB to the canonical view volume, such that the bounds in each dimension are [-1,1].
    
    return Mat44f{ 
        {2.f/(r-l),     0.f,            0.f,           0.f},
        {0.f,           2.f/(t-b),      0.f,           0.f},
        {0.f,           0.f,            -2.f/(f-n),    0.f},
        {-(r+l)/(r-l),  -(t+b)/(t-b),   -(f+n)/(f-n),  1.f}
    };
}



/// @brief Constructs a perspective projection matrix
/// @brief Assumes that the viewer oriented in a right-handed coordinate system, looking towards the -z direction, with the y axis pointing up. 
/// @param vfov vertical field-of-view in radians
/// @param near z-coordinate of near plane (< 0)
/// @param far z-coordinate of far plane (< n)
/// @return Matrix representing the associated projection
[[nodiscard]] inline Mat44f  PerspectiveProjection(float vfov, float  aspect, float near, float far)
{
    //assert(0>n>f)
    const float f = 1.f / std::tan(0.5f * vfov);
    return Mat44f{
        {f/aspect,     0.f,  0.f,                          0.f},
        {0.f,          f,    0.f,                          0.f},
        {0.f,          0.f,  -(far+near)/(far-near),      -1.f},
        {0.f,          0.f,  -(2.f*far*near)/(far-near),   0.f}        
        };
}



// /// @brief Applies the viewport transform 
// /// @param ndc_coords Coordinates inside the unit cube [-1,1]
// /// @param im_height 
// /// @param im_width 
// /// @param flip_y 
// /// @return 
// [[nodiscard]] Vec3f ViewPort(const Vec3f& ndc_coords, int im_height, int im_width, bool flip_y) {

//     const auto y_scale{flip_y ? -1.f : 1.f};
//     const auto x_pixel = (ndc_coords.X()+1)*im_width/2.f;
//     const auto y_pixel = (y_scale*ndc_coords.Y()+1)*im_height/2.f;
//     return Vec3f(x_pixel,y_pixel, ndc_coords.Z());
// }