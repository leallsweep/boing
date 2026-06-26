# Boink 3D Engine

A minimal 3D engine written in C++ with Lua scripting, an OpenGL renderer, and a built-in window.

---

## Features

| Feature | Details |
|---------|---------|
| OpenGL 3.3 window | GLFW + GLEW |
| Shapes | Cube, Plane (with subdivisions), Triangle |
| Camera | FPS movement (WASD), orbital (arrow keys) |
| Scripting | Lua 5.3/5.4 — script file loaded at startup |
| Lighting | Diffuse + ambient (hardcoded directional light) |
| C++ API | Build scenes directly without Lua |

---

## Dependencies

```
OpenGL 3.3+
GLEW
GLFW3
Lua 5.3 or 5.4
CMake 3.16+
```

### Ubuntu / Debian
```bash
sudo apt install libglew-dev libglfw3-dev liblua5.4-dev cmake build-essential
```

### macOS (Homebrew)
```bash
brew install glew glfw lua cmake
```

### Windows (vcpkg)
```bash
vcpkg install glew glfw3 lua cmake
```

---

## Building

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . -j$(nproc)
```

---

## Running

```bash
# Default script (scripts/scene.lua)
./boink

# Custom script
./boink my_scene.lua
```

---

## Controls (built-in)

| Key | Action |
|-----|--------|
| `W/S` | Move forward / backward |
| `A/D` | Strafe left / right |
| `Q/E` | Move up / down |
| `←→` | Orbit around target (horizontal) |
| `↑↓` | Move camera up / down |
| `Esc` | Quit |

---

## Lua API

### Creating shapes

```lua
-- Cube
local cube = Boink.addCube("myCube")

-- Plane (second argument = subdivision count, default 1)
local ground = Boink.addPlane("ground", 10)

-- Triangle
local tri = Boink.addTriangle("tri")
```

### Shape methods (chainable)

```lua
cube:moveTo(x, y, z)           -- teleport to position
cube:move(dx, dy, dz)          -- move relative to current position
cube:rotateTo(rx, ry, rz)      -- set Euler angles (radians)
cube:rotate(dx, dy, dz)        -- add to current rotation
cube:scaleTo(sx, sy, sz)       -- set scale
cube:setColor(r, g, b, a)      -- color (0..1), a is optional
cube:setWireframe(true/false)   -- toggle wireframe rendering
cube:getName()                  -- → string
cube:getType()                  -- → "Cube" | "Plane" | "Triangle"
cube:getPosition()              -- → x, y, z (three return values)
```

### Camera

```lua
Boink.cameraMoveTo(x, y, z)    -- teleport camera
Boink.cameraMove(dx, dy, dz)   -- move camera by offset
Boink.cameraLookAt(x, y, z)    -- set look-at target
Boink.cameraSetFov(degrees)     -- set field of view
Boink.cameraGetPosition()       -- → x, y, z
```

### Accessing shapes by name

```lua
local s = Boink.getShape("myCube")   -- returns nil if not found
Boink.removeShape("myCube")           -- remove shape from scene
```

### Callbacks

```lua
function onLoad()
    -- called once when the script is first loaded
end

function onUpdate(dt)
    -- dt = delta time in seconds, called every frame
end
```

---

## C++ API (without Lua)

```cpp
#include "boink/include/boink.h"

int main(){
    Boink::WindowConfig cfg;
    cfg.width  = 1280;
    cfg.height = 720;
    cfg.title  = "Boink";

    Boink::Window window(cfg);
    auto& scene = window.scene();

    // Camera
    scene.cameraMoveTo(0, 3, 8);
    scene.cameraLookAt(0, 0, 0);

    // Shapes
    auto* cube = scene.addCube("cube");
    cube->setColor(0.2f, 0.6f, 1.0f);

    auto* ground = scene.addPlane("ground", 10);
    ground->scaleTo(10, 1, 10);
    ground->moveTo(0, -1, 0);
    ground->setColor(0.3f, 0.5f, 0.3f);

    auto* tri = scene.addTriangle("tri");
    tri->moveTo(-2, 0, 0);
    tri->setColor(1.0f, 0.85f, 0.1f);

    // Per-frame C++ callback
    window.setUpdateCallback([](Boink::Scene& s, float dt){
        static float t = 0; t += dt;
        if(auto* c = s.get("cube")) c->rotateTo(t * 0.5f, t, 0);
    });

    // Or load a Lua script instead
    window.loadScript("my_script.lua");

    window.run();  // blocks until the window is closed
}
```

---

## Project structure

```
boink/
├── include/
│   ├── boink.h           ← single include for user code
│   ├── boink_math.h      ← Vec3, Mat4
│   ├── boink_camera.h    ← Camera
│   ├── boink_shapes.h    ← Shape (base), Cube, Plane, Triangle
│   ├── boink_scene.h     ← Scene — manages all objects
│   ├── boink_renderer.h  ← OpenGL renderer
│   ├── boink_script.h    ← Lua scripting engine
│   └── boink_window.h    ← GLFW window + main loop
├── src/
│   ├── boink_renderer.cpp
│   ├── boink_script.cpp
│   └── boink_window.cpp
├── scripts/
│   └── scene.lua         ← example Lua scene
├── main.cpp              ← entry point
└── CMakeLists.txt
```

---

## Extending: adding a new shape

1. Create a class inheriting from `Boink::Shape`:

```cpp
class Sphere : public Shape {
public:
    Sphere(int stacks = 16, int slices = 16) {
        // fill vertices and indices here
        indexCount = (int)indices.size();
    }
    std::string type() const override { return "Sphere"; }
};
```

2. Add a factory method to `Scene`:

```cpp
Shape* addSphere(const std::string& name, int stacks = 16, int slices = 16) {
    auto s = std::make_shared<Sphere>(stacks, slices);
    s->name = name;
    return store(name, s);
}
```

3. Register it in Lua inside `boink_script.cpp`:

```cpp
static int lua_addSphere(lua_State* L) {
    pushShape(L, getScene(L)->addSphere(luaL_checkstring(L, 1)));
    return 1;
}
// In registerAPI(): reg("addSphere", lua_addSphere);
```

Done — `Boink.addSphere("ball")` now works in Lua.
