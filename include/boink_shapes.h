#pragma once
#include "boink_math.h"
#include <vector>
#include <string>
#include <cstdint>

namespace Boink {

// ─── Color ───────────────────────────────────────────────────────────────────
struct Color { float r=1, g=1, b=1, a=1; };

// ─── Shape (abstract base) ───────────────────────────────────────────────────
class Shape {
public:
    std::string name;
    Vec3  position = {0,0,0};
    Vec3  rotation = {0,0,0};  // Euler angles in radians
    Vec3  scale    = {1,1,1};
    Color color    = {1,1,1,1};

    // GPU handles (set by renderer)
    unsigned int vao = 0;
    unsigned int vbo = 0;
    unsigned int ebo = 0;
    int  indexCount  = 0;
    bool wireframe   = false;

    virtual ~Shape() = default;
    virtual std::string type() const = 0;

    // CPU mesh data to be uploaded to GPU
    std::vector<float>        vertices; // x y z  nx ny nz
    std::vector<unsigned int> indices;

    void moveTo(float x, float y, float z) { position = {x,y,z}; }
    void move  (float x, float y, float z) { position += Vec3{x,y,z}; }
    void rotateTo(float x, float y, float z) { rotation = {x,y,z}; }
    void rotate  (float x, float y, float z) { rotation += Vec3{x,y,z}; }
    void scaleTo(float x, float y, float z) { scale = {x,y,z}; }
    void setColor(float r, float g, float b, float a=1) { color={r,g,b,a}; }
    void setWireframe(bool w) { wireframe = w; }

    Mat4 modelMatrix() const {
        return Mat4::translate(position)
             * Mat4::rotateY(rotation.y)
             * Mat4::rotateX(rotation.x)
             * Mat4::rotateZ(rotation.z)
             * Mat4::scale(scale);
    }

protected:
    void addVertex(float x,float y,float z,float nx,float ny,float nz){
        vertices.insert(vertices.end(), {x,y,z,nx,ny,nz});
    }
    void addTri(uint32_t a,uint32_t b,uint32_t c){
        indices.insert(indices.end(), {a,b,c});
    }
    void addQuad(uint32_t a,uint32_t b,uint32_t c,uint32_t d){
        addTri(a,b,c); addTri(a,c,d);
    }
};

// ─── Triangle ────────────────────────────────────────────────────────────────
class Triangle : public Shape {
public:
    Triangle() {
        // unit equilateral in XY plane
        addVertex(-0.5f,-0.289f,0,  0,0,1);
        addVertex( 0.5f,-0.289f,0,  0,0,1);
        addVertex( 0.0f, 0.577f,0,  0,0,1);
        addTri(0,1,2);
        indexCount = 3;
    }
    std::string type() const override { return "Triangle"; }
};

// ─── Plane ───────────────────────────────────────────────────────────────────
class Plane : public Shape {
public:
    explicit Plane(int subdivisions = 1) {
        int n = subdivisions + 1;
        float step = 1.0f / subdivisions;
        for(int z=0; z<=subdivisions; z++)
          for(int x=0; x<=subdivisions; x++){
            float px = (x*step - 0.5f);
            float pz = (z*step - 0.5f);
            addVertex(px, 0, pz,  0,1,0);
          }
        for(int z=0; z<subdivisions; z++)
          for(int x=0; x<subdivisions; x++){
            int base = z*n+x;
            addQuad(base, base+1, base+n+1, base+n);
          }
        indexCount = (int)indices.size();
    }
    std::string type() const override { return "Plane"; }
};

// ─── Cube ─────────────────────────────────────────────────────────────────────
class Cube : public Shape {
public:
    Cube() {
        // 6 faces, each 4 vertices
        auto face = [&](Vec3 n, Vec3 r, Vec3 u){
            Vec3 right = r, up2 = u;
            uint32_t base = (uint32_t)(vertices.size()/6);
            for(int i=0;i<4;i++){
                float s = (i&1) ? 1.f : -1.f;
                float t = (i&2) ? 1.f : -1.f;
                Vec3 p = n + right*s + up2*t;
                addVertex(p.x*0.5f, p.y*0.5f, p.z*0.5f,  n.x, n.y, n.z);
            }
            // quad: 0,1,3,2
            addQuad(base, base+1, base+3, base+2);
        };
        face({ 0, 0, 1},{ 1, 0, 0},{ 0, 1, 0}); // front
        face({ 0, 0,-1},{-1, 0, 0},{ 0, 1, 0}); // back
        face({ 1, 0, 0},{ 0, 0,-1},{ 0, 1, 0}); // right
        face({-1, 0, 0},{ 0, 0, 1},{ 0, 1, 0}); // left
        face({ 0, 1, 0},{ 1, 0, 0},{ 0, 0,-1}); // top
        face({ 0,-1, 0},{ 1, 0, 0},{ 0, 0, 1}); // bottom
        indexCount = (int)indices.size();
    }
    std::string type() const override { return "Cube"; }
};

} // namespace Boink
