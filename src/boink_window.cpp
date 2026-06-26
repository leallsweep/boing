#include "../include/boink_window.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <cstdio>
#include <unordered_set>
#include <chrono>

namespace Boink {

static Window* g_win = nullptr; // for GLFW callbacks

void framebufferCB(GLFWwindow*, int w, int h){
    if(g_win){ g_win->m_width=w; g_win->m_height=h; }
    glViewport(0,0,w,h);
}

Window::Window(const WindowConfig& cfg)
    : m_cfg(cfg), m_width(cfg.width), m_height(cfg.height)
{
    g_win = this;

    if(!glfwInit()){
        fprintf(stderr,"[Boink] glfwInit failed\n");
        return;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, cfg.resizable ? GLFW_TRUE : GLFW_FALSE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    m_glfwWin = glfwCreateWindow(cfg.width, cfg.height,
                                  cfg.title.c_str(), nullptr, nullptr);
    if(!m_glfwWin){
        fprintf(stderr,"[Boink] glfwCreateWindow failed\n");
        glfwTerminate(); return;
    }
    glfwMakeContextCurrent(m_glfwWin);
    glfwSwapInterval(cfg.vsync ? 1 : 0);
    glfwSetFramebufferSizeCallback(m_glfwWin, framebufferCB);

    m_renderer.init(cfg.width, cfg.height);
    m_scripts.bindScene(&m_scene);
}

Window::~Window(){
    if(m_glfwWin) glfwDestroyWindow(m_glfwWin);
    glfwTerminate();
}

bool Window::loadScript(const std::string& path){
    std::string err;
    if(!m_scripts.execFile(path, err)){
        fprintf(stderr,"[Boink] Script error: %s\n", err.c_str());
        return false;
    }
    m_hasScript = true;
    // call onLoad if present
    m_scripts.callFunction("onLoad", 0.0f, err);
    return true;
}

void Window::setUpdateCallback(std::function<void(Scene&, float)> cb){
    m_updateCB = std::move(cb);
}

bool Window::isOpen() const {
    return m_glfwWin && !glfwWindowShouldClose(m_glfwWin);
}

// ── Input (WASD + arrows + mouse look stub) ───────────────────────────────────
void Window::processInput(float dt){
    if(!m_glfwWin) return;
    auto& cam = m_scene.camera;
    float spd = 5.0f * dt;

    // forward/back direction
    Vec3 fwd = (cam.target - cam.position).normalized();
    Vec3 right = fwd.cross(cam.up).normalized();

    if(glfwGetKey(m_glfwWin, GLFW_KEY_ESCAPE)==GLFW_PRESS)
        glfwSetWindowShouldClose(m_glfwWin, true);

    if(glfwGetKey(m_glfwWin, GLFW_KEY_W)==GLFW_PRESS) cam.move(fwd*spd);
    if(glfwGetKey(m_glfwWin, GLFW_KEY_S)==GLFW_PRESS) cam.move(fwd*(-spd));
    if(glfwGetKey(m_glfwWin, GLFW_KEY_A)==GLFW_PRESS) cam.move(right*(-spd));
    if(glfwGetKey(m_glfwWin, GLFW_KEY_D)==GLFW_PRESS) cam.move(right*spd);
    if(glfwGetKey(m_glfwWin, GLFW_KEY_Q)==GLFW_PRESS) cam.move(cam.up*spd);
    if(glfwGetKey(m_glfwWin, GLFW_KEY_E)==GLFW_PRESS) cam.move(cam.up*(-spd));

    // Arrow keys: orbit target
    float orb = 1.5f * dt;
    Vec3 diff  = cam.position - cam.target;
    if(glfwGetKey(m_glfwWin, GLFW_KEY_LEFT)==GLFW_PRESS){
        float c=cosf(orb),s=sinf(orb);
        Vec3 d2{diff.x*c+diff.z*s, diff.y, -diff.x*s+diff.z*c};
        cam.position = cam.target + d2;
    }
    if(glfwGetKey(m_glfwWin, GLFW_KEY_RIGHT)==GLFW_PRESS){
        float c=cosf(-orb),s=sinf(-orb);
        Vec3 d2{diff.x*c+diff.z*s, diff.y, -diff.x*s+diff.z*c};
        cam.position = cam.target + d2;
    }
    if(glfwGetKey(m_glfwWin, GLFW_KEY_UP)==GLFW_PRESS){
        cam.position.y += spd;
    }
    if(glfwGetKey(m_glfwWin, GLFW_KEY_DOWN)==GLFW_PRESS){
        cam.position.y -= spd;
    }
}

// Track uploaded shapes
static std::unordered_set<Shape*> g_uploaded;

void Window::uploadNewShapes(){
    for(auto& sp : m_scene.shapes()){
        Shape* s = sp.get();
        if(g_uploaded.count(s)==0){
            m_renderer.uploadShape(s);
            g_uploaded.insert(s);
        }
    }
}

void Window::run(){
    if(!m_glfwWin) return;

    auto last = std::chrono::high_resolution_clock::now();

    while(!glfwWindowShouldClose(m_glfwWin)){
        // dt
        auto now = std::chrono::high_resolution_clock::now();
        float dt = std::chrono::duration<float>(now-last).count();
        last = now;

        glfwPollEvents();
        processInput(dt);

        // tick Lua onUpdate
        if(m_hasScript){
            std::string err;
            if(!m_scripts.callFunction("onUpdate", dt, err))
                fprintf(stderr,"[Boink] onUpdate error: %s\n", err.c_str());
        }

        // C++ callback
        if(m_updateCB) m_updateCB(m_scene, dt);

        // Upload any newly added shapes
        uploadNewShapes();

        // Render
        m_renderer.clear();
        m_renderer.drawAll(m_scene.shapes(), m_scene.camera, m_width, m_height);
        glfwSwapBuffers(m_glfwWin);
    }
}

} // namespace Boink
