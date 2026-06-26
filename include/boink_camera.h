#pragma once
#include "boink_math.h"

namespace Boink {

class Camera {
public:
    Vec3  position  = {0, 0, 5};
    Vec3  target    = {0, 0, 0};
    Vec3  up        = {0, 1, 0};
    float fov       = 60.0f;   // degrees
    float nearPlane = 0.1f;
    float farPlane  = 1000.0f;

    // Move camera relative to its own axes
    void move(Vec3 delta) { position += delta; target += delta; }

    void setPosition(float x, float y, float z) {
        Vec3 off = Vec3{x,y,z} - position;
        position = {x,y,z};
        target   = target + off;
    }
    void setTarget(float x, float y, float z) { target = {x,y,z}; }
    void setFov(float f)                       { fov = f; }
    void setUp(float x, float y, float z)      { up = {x,y,z}; }

    Mat4 viewMatrix() const {
        return Mat4::lookAt(position, target, up);
    }
    Mat4 projectionMatrix(float aspect) const {
        float fovRad = fov * 3.14159265f / 180.0f;
        return Mat4::perspective(fovRad, aspect, nearPlane, farPlane);
    }
};

} // namespace Boink
