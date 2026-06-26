#pragma once
#include "boink_math.h"
#include "boink_texture.h"
#include <vector>
#include <string>
#include <cstdint>

namespace Boink {

// ─── Color ───────────────────────────────────────────────────────────────────
struct Color { float r=1, g=1, b=1, a=1; };

// ─── Shape (abstract base) ───────────────────────────────────────────────────
// Vertex layout: x y z  nx ny nz  u v  (8 floats)
class Shape {
public:
    std::string name;
    Vec3  position = {0,0,0};
    Vec3  rotation = {0,0,0};  // Euler angles in radians
    Vec3  scale    = {1,1,1};
    Color color    = {1,1,1,1};

    // Texture (nullptr = use flat color)
    Texture* texture     = nullptr;
    float    textureTile = 1.0f;   // UV tiling multiplier

    // GPU handles (set by renderer)
    unsigned int vao = 0;
    unsigned int vbo = 0;
    unsigned int ebo = 0;
    int  indexCount  = 0;
    bool wireframe   = false;

    virtual ~Shape() = default;
    virtual std::string type() const = 0;

    // CPU mesh data — 8 floats per vertex: pos(3) normal(3) uv(2)
    std::vector<float>        vertices;
    std::vector<unsigned int> indices;

    void moveTo(float x, float y, float z) { position = {x,y,z}; }
    void move  (float x, float y, float z) { position += Vec3{x,y,z}; }
    void rotateTo(float x, float y, float z) { rotation = {x,y,z}; }
    void rotate  (float x, float y, float z) { rotation += Vec3{x,y,z}; }
    void scaleTo(float x, float y, float z) { scale = {x,y,z}; }
    void setColor(float r, float g, float b, float a=1) { color={r,g,b,a}; }
    void setWireframe(bool w) { wireframe = w; }
    void setTexture(Texture* t)         { texture = t; }
    void setTextureTile(float tile)     { textureTile = tile; }

    Mat4 modelMatrix() const {
        return Mat4::translate(position)
             * Mat4::rotateY(rotation.y)
             * Mat4::rotateX(rotation.x)
             * Mat4::rotateZ(rotation.z)
             * Mat4::scale(scale);
    }

protected:
    // pos + normal + uv
    void addVertex(float x, float y, float z,
                   float nx, float ny, float nz,
                   float u,  float v) {
        vertices.insert(vertices.end(), {x,y,z, nx,ny,nz, u,v});
    }
    void addTri(uint32_t a, uint32_t b, uint32_t c){
        indices.insert(indices.end(), {a,b,c});
    }
    void addQuad(uint32_t a, uint32_t b, uint32_t c, uint32_t d){
        addTri(a,b,c); addTri(a,c,d);
    }
};

// ─── Triangle ────────────────────────────────────────────────────────────────
class Triangle : public Shape {
public:
    Triangle() {
        // unit equilateral in XY plane
        addVertex(-0.5f,-0.289f,0,  0,0,1,  0.0f,0.0f);
        addVertex( 0.5f,-0.289f,0,  0,0,1,  1.0f,0.0f);
        addVertex( 0.0f, 0.577f,0,  0,0,1,  0.5f,1.0f);
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
            float px = x*step - 0.5f;
            float pz = z*step - 0.5f;
            float u  = (float)x / subdivisions;
            float v  = (float)z / subdivisions;
            addVertex(px, 0, pz,  0,1,0,  u,v);
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
        // Each face: 4 vertices with proper UVs
        //   v indices per quad:  0=(-s,-t)  1=(+s,-t)  2=(+s,+t)  3=(-s,+t)
        auto face = [&](Vec3 n, Vec3 right, Vec3 up){
            uint32_t base = (uint32_t)(vertices.size() / 8);
            // corners in (s,t) space: (-1,-1),(+1,-1),(+1,+1),(-1,+1)
            float cs[4] = {-1, 1, 1,-1};
            float ts[4] = {-1,-1, 1, 1};
            float us[4] = { 0, 1, 1, 0};
            float vs2[4]= { 0, 0, 1, 1};
            for(int i=0;i<4;i++){
                Vec3 p = n + right*cs[i] + up*ts[i];
                addVertex(p.x*0.5f, p.y*0.5f, p.z*0.5f,
                          n.x, n.y, n.z,
                          us[i], vs2[i]);
            }
            // quad: 0,1,2,3 -> tris 0,1,2 and 0,2,3
            addTri(base, base+1, base+2);
            addTri(base, base+2, base+3);
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
