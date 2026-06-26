#pragma once
#include <cmath>
#include <string>

namespace Boink {

// ─── Vec3 ────────────────────────────────────────────────────────────────────
struct Vec3 {
    float x = 0, y = 0, z = 0;
    Vec3() = default;
    Vec3(float x, float y, float z) : x(x), y(y), z(z) {}

    Vec3 operator+(const Vec3& o) const { return {x+o.x, y+o.y, z+o.z}; }
    Vec3 operator-(const Vec3& o) const { return {x-o.x, y-o.y, z-o.z}; }
    Vec3 operator*(float s)       const { return {x*s,   y*s,   z*s};   }
    Vec3& operator+=(const Vec3& o){ x+=o.x; y+=o.y; z+=o.z; return *this; }
    Vec3& operator-=(const Vec3& o){ x-=o.x; y-=o.y; z-=o.z; return *this; }

    float  dot(const Vec3& o)  const { return x*o.x + y*o.y + z*o.z; }
    Vec3   cross(const Vec3& o)const {
        return { y*o.z - z*o.y, z*o.x - x*o.z, x*o.y - y*o.x };
    }
    float  length()            const { return std::sqrt(x*x + y*y + z*z); }
    Vec3   normalized()        const {
        float l = length();
        return l > 1e-6f ? Vec3{x/l, y/l, z/l} : Vec3{0,0,0};
    }
};

// ─── Mat4 (column-major) ─────────────────────────────────────────────────────
struct Mat4 {
    float m[16] = {};

    Mat4() { identity(); }

    void identity() {
        for(int i=0;i<16;i++) m[i]=0;
        m[0]=m[5]=m[10]=m[15]=1;
    }

    const float* data() const { return m; }

    // matrix * matrix
    Mat4 operator*(const Mat4& o) const {
        Mat4 r; for(int i=0;i<16;i++) r.m[i]=0;
        for(int row=0;row<4;row++)
          for(int col=0;col<4;col++)
            for(int k=0;k<4;k++)
              r.m[col*4+row] += m[k*4+row] * o.m[col*4+k];
        return r;
    }

    static Mat4 translate(Vec3 t) {
        Mat4 r;
        r.m[12]=t.x; r.m[13]=t.y; r.m[14]=t.z;
        return r;
    }
    static Mat4 scale(Vec3 s) {
        Mat4 r;
        r.m[0]=s.x; r.m[5]=s.y; r.m[10]=s.z;
        return r;
    }
    static Mat4 rotateX(float a) {
        Mat4 r;
        r.m[5]=std::cos(a); r.m[6]=std::sin(a);
        r.m[9]=-std::sin(a);r.m[10]=std::cos(a);
        return r;
    }
    static Mat4 rotateY(float a) {
        Mat4 r;
        r.m[0]=std::cos(a); r.m[2]=-std::sin(a);
        r.m[8]=std::sin(a); r.m[10]=std::cos(a);
        return r;
    }
    static Mat4 rotateZ(float a) {
        Mat4 r;
        r.m[0]=std::cos(a);  r.m[1]=std::sin(a);
        r.m[4]=-std::sin(a); r.m[5]=std::cos(a);
        return r;
    }
    static Mat4 perspective(float fovY, float aspect, float near, float far) {
        Mat4 r; for(int i=0;i<16;i++) r.m[i]=0;
        float f = 1.0f / std::tan(fovY * 0.5f);
        r.m[0]  = f / aspect;
        r.m[5]  = f;
        r.m[10] = (far + near) / (near - far);
        r.m[11] = -1.0f;
        r.m[14] = (2.0f * far * near) / (near - far);
        return r;
    }
    static Mat4 lookAt(Vec3 eye, Vec3 center, Vec3 up) {
        Vec3 f = (center - eye).normalized();
        Vec3 r = f.cross(up).normalized();
        Vec3 u = r.cross(f);
        Mat4 m;
        m.m[0]=r.x; m.m[4]=r.y; m.m[8]=r.z;  m.m[12]=-r.dot(eye);
        m.m[1]=u.x; m.m[5]=u.y; m.m[9]=u.z;  m.m[13]=-u.dot(eye);
        m.m[2]=-f.x;m.m[6]=-f.y;m.m[10]=-f.z;m.m[14]=f.dot(eye);
        m.m[3]=0;   m.m[7]=0;   m.m[11]=0;    m.m[15]=1;
        return m;
    }
};

} // namespace Boink
