#pragma once
#include "boink_shapes.h"
#include "boink_camera.h"
#include <memory>
#include <vector>
#include <string>
#include <unordered_map>
#include <algorithm>

namespace Boink {

class Scene {
public:
    Camera camera;

    // ── Shape factory ───────────────────────────────────────────────────────
    Shape* addCube(const std::string& name) {
        auto s = std::make_shared<Cube>();
        s->name = name;
        return store(name, s);
    }
    Shape* addPlane(const std::string& name, int subdivisions=1) {
        auto s = std::make_shared<Plane>(subdivisions);
        s->name = name;
        return store(name, s);
    }
    Shape* addTriangle(const std::string& name) {
        auto s = std::make_shared<Triangle>();
        s->name = name;
        return store(name, s);
    }

    Shape* get(const std::string& name) {
        auto it = m_index.find(name);
        return it != m_index.end() ? it->second.get() : nullptr;
    }
    bool remove(const std::string& name) {
        auto it = m_index.find(name);
        if(it == m_index.end()) return false;
        Shape* raw = it->second.get();
        m_shapes.erase(std::remove_if(m_shapes.begin(), m_shapes.end(),
            [&](const std::shared_ptr<Shape>& p){ return p.get() == raw; }),
            m_shapes.end());
        m_index.erase(it);
        return true;
    }

    const std::vector<std::shared_ptr<Shape>>& shapes() const { return m_shapes; }

    // ── Camera convenience ─────────────────────────────────────────────────
    void cameraMoveTo(float x,float y,float z)  { camera.setPosition(x,y,z); }
    void cameraMove  (float x,float y,float z)  { camera.move({x,y,z}); }
    void cameraLookAt(float x,float y,float z)  { camera.setTarget(x,y,z); }
    void cameraSetFov(float fov)                 { camera.setFov(fov); }

private:
    std::vector<std::shared_ptr<Shape>>           m_shapes;
    std::unordered_map<std::string,std::shared_ptr<Shape>> m_index;

    Shape* store(const std::string& name, std::shared_ptr<Shape> s) {
        m_index[name] = s;
        m_shapes.push_back(s);
        return s.get();
    }
};

} // namespace Boink
