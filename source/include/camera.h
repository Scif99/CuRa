#ifndef CAMERA_H
#define CAMERA_H

#include "vec.h"

struct Camera{

    Camera(const Vec3f& e, const Vec3f& c, const Vec3f& u)
        : eye{e}, center{c}, up{u} {}

    Camera(Vec3f&& e, Vec3f&& c, Vec3f&& u)
        : eye{std::move(e)}, center{std::move(c)}, up{std::move(u)} {}

    Vec3f eye; //Position of the camera
    Vec3f center; //Direction the camera is looking
    Norm3f up; //Defines orientation of the camera 
};

#endif