#pragma once
#include "boink_shapes.h"
#include "boink_camera.h"
#include <memory>
#include <vector>

namespace Boink {

class Renderer {
public:
    Renderer()  = default;
    ~Renderer() = default;

    bool init(int width, int height);   // compile shaders, etc.
    void resize(int width, int height);
    void clear(float r=0.07f, float g=0.07f, float b=0.1f);
    void uploadShape(Shape* shape);     // send VBO/EBO to GPU
    void draw(Shape* shape, const Camera& cam, int vpW, int vpH);
    void drawAll(const std::vector<std::shared_ptr<Shape>>& shapes,
                 const Camera& cam, int vpW, int vpH);

private:
    unsigned int m_shaderProg = 0;
    int m_uMVP   = -1;
    int m_uModel = -1;
    int m_uColor = -1;
    int m_uLightDir = -1;
    int m_uAmbient  = -1;

    unsigned int compileShader(unsigned int type, const char* src);
};

} // namespace Boink
