#pragma once

#include <cura/math.h>
#include <cura/transforms.h>

//Note that we save the view matrix here so that it only needs to be computed once
struct Camera{
    Camera(const Vec3f& e, const Vec3f& c, const Vec3f& u)
        : eye{e}, center{c}, up{la::normalize(u)} 
        {
            view = LookAt(eye, center, up);
        }

    Camera(Vec3f&& e, Vec3f&& c, Vec3f&& u)
        : eye{std::move(e)}, center{std::move(c)}, up{std::move(la::normalize(u))} 
        {
            view = LookAt(eye, center, up);
        }

    Vec3f eye; //Position of the camera.
    Vec3f center; //Direction the camera is looking.
    Vec3f up; //Defines orientation of the camera .
    Mat44f view; //Store the world-to-camera change of basis matrix.
};
