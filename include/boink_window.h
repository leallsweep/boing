#pragma once
#include "boink_scene.h"
#include "boink_renderer.h"
#include "boink_script.h"
#include <string>
#include <functional>

// Forward declare GLFW
struct GLFWwindow;

namespace Boink {

struct WindowConfig {
    int         width       = 1024;
    int         height      = 768;
    std::string title       = "Boink";
    bool        vsync       = true;
    bool        resizable   = true;
};

class Window {
public:
    explicit Window(const WindowConfig& cfg = {});
    ~Window();

    // Load a Lua script that will be called each frame
    bool loadScript(const std::string& path);

    // Override per-frame callback from C++ side
    void setUpdateCallback(std::function<void(Scene&, float dt)> cb);

    // Run the main loop (blocks until closed)
    void run();

    // Access the scene from C++
    Scene& scene() { return m_scene; }

    bool isOpen() const;

private:
    WindowConfig m_cfg;
    GLFWwindow*  m_glfwWin  = nullptr;
    Scene        m_scene;
    Renderer     m_renderer;
    ScriptEngine m_scripts;
    bool         m_hasScript = false;

    std::function<void(Scene&, float)> m_updateCB;

    void initGL();
    void processInput(float dt);
    void uploadNewShapes();

    int m_width, m_height;

    friend void framebufferCB(GLFWwindow*, int, int);

    // set of shapes already uploaded
    struct ShapeState { unsigned int vao=0; };
};

} // namespace Boink
