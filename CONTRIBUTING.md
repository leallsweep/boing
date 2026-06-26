# Contributing to Boink

Thanks for your interest in contributing! This document covers how the codebase is structured, what the conventions are, and how to get a change merged.

---

## Table of contents

- [Getting started](#getting-started)
- [Project layout](#project-layout)
- [Building for development](#building-for-development)
- [Coding conventions](#coding-conventions)
- [Adding a new shape](#adding-a-new-shape)
- [Extending the Lua API](#extending-the-lua-api)
- [Submitting a pull request](#submitting-a-pull-request)
- [Reporting bugs](#reporting-bugs)

---

## Getting started

1. Fork the repository and clone your fork.
2. Install the dependencies listed in [README.md](README.md#dependencies).
3. Build the project in debug mode (see below) and make sure it compiles and runs cleanly before touching anything.

---

## Project layout

```
boink/
├── include/          ← public headers — this is the API surface
│   ├── boink.h       ← single include for downstream users
│   ├── boink_math.h  ← Vec3, Mat4 (no external dependencies)
│   ├── boink_camera.h
│   ├── boink_shapes.h
│   ├── boink_scene.h
│   ├── boink_renderer.h
│   ├── boink_script.h
│   └── boink_window.h
├── src/              ← implementation files
│   ├── boink_renderer.cpp   (OpenGL + GLSL shaders)
│   ├── boink_script.cpp     (Lua bridge)
│   └── boink_window.cpp     (GLFW window + main loop)
├── scripts/          ← example and default Lua scenes
├── main.cpp          ← entry point / host application
└── CMakeLists.txt
```

**Rule of thumb:** if something is part of the public API, it belongs in `include/`. If it is an implementation detail, it stays in `src/`.

---

## Building for development

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
cmake --build . -j$(nproc)
```

Debug builds enable assertions and keep symbols for stack traces. Always develop and test against a debug build before switching to `Release`.

---

## Coding conventions

Boink follows a small, consistent set of rules. Please match the style of the surrounding code rather than introducing a new style in your patch.

### General

- **C++17** — use modern features freely (`if constexpr`, structured bindings, `std::optional`, etc.), but avoid anything that requires a newer standard.
- Keep files focused. One logical concept per header.
- No external dependencies beyond OpenGL, GLEW, GLFW, and Lua. If you think an exception is warranted, open an issue first.

### Naming

| Thing | Convention | Example |
|-------|-----------|---------|
| Types / classes | `PascalCase` | `ShapeState` |
| Member variables | `m_camelCase` | `m_shaderProg` |
| Free functions | `camelCase` | `compileShader` |
| Lua-facing C functions | `lua_camelCase` | `lua_addCube` |
| Constants / macros | `UPPER_SNAKE` | `SCENE_KEY` |
| Files | `boink_snake_case` | `boink_shapes.h` |

### Formatting

- 4-space indentation, no tabs.
- Opening braces on the same line for functions and control flow.
- Keep lines under 100 characters where reasonable.
- Section dividers use the existing style:

```cpp
// ─── Section name ────────────────────────────────────────────────────────────
```

### Error handling

- Use `fprintf(stderr, "[Boink] ...")` for runtime errors in C++ code — no exceptions.
- Lua errors must go through `lua_error` (or `luaL_error`) — never let a C++ exception cross the Lua boundary.
- Return `bool` + an `std::string& errOut` for operations that can fail and need to report why (see `ScriptEngine::execFile`).

### Math

- All angles are in **radians** internally. Degrees are only used at the API boundary (e.g. `Camera::fov`) and converted immediately.
- `Vec3` and `Mat4` live in `boink_math.h` and have no external dependencies. Keep it that way.

---

## Adding a new shape

1. **Define the class** in `include/boink_shapes.h`, inheriting from `Shape`:

```cpp
// ─── Sphere ───────────────────────────────────────────────────────────────────
class Sphere : public Shape {
public:
    explicit Sphere(int stacks = 16, int slices = 16) {
        // populate vertices (x y z nx ny nz) and indices
        // ...
        indexCount = (int)indices.size();
    }
    std::string type() const override { return "Sphere"; }
};
```

2. **Add a factory method** to `Scene` in `include/boink_scene.h`:

```cpp
Shape* addSphere(const std::string& name, int stacks = 16, int slices = 16) {
    auto s = std::make_shared<Sphere>(stacks, slices);
    s->name = name;
    return store(name, s);
}
```

3. **Register it in Lua** in `src/boink_script.cpp`:

```cpp
static int lua_addSphere(lua_State* L) {
    Scene* sc = getScene(L);
    const char* name = luaL_checkstring(L, 1);
    int stacks = lua_isnumber(L, 2) ? (int)lua_tointeger(L, 2) : 16;
    int slices = lua_isnumber(L, 3) ? (int)lua_tointeger(L, 3) : 16;
    pushShape(L, sc->addSphere(name, stacks, slices));
    return 1;
}
// Inside registerAPI():
reg("addSphere", lua_addSphere);
```

4. **Add an example** to `scripts/scene.lua` showing the new shape in action.

The renderer uploads any shape automatically — you do not need to touch `boink_renderer.cpp` for a standard triangulated mesh.

---

## Extending the Lua API

All Lua bindings live in `src/boink_script.cpp`. The pattern is always the same:

```cpp
// 1. Write a static C function with signature int(lua_State*)
static int lua_myNewFunction(lua_State* L) {
    // read arguments with luaL_check* / lua_to*
    // call C++ API
    // push return values
    return <number of return values>;
}

// 2. Register it inside ScriptEngine::registerAPI()
reg("myNewFunction", lua_myNewFunction);
```

After that, the function is available in Lua as `Boink.myNewFunction(...)`.

A few rules for Lua bindings:

- Validate every argument with `luaL_check*`. Never assume correct types — bad Lua scripts should produce a clear error, not a crash.
- Return the shape userdata (`lua_pushvalue(L, 1); return 1;`) from mutating shape methods so calls can be chained in Lua.
- Do not allocate heap memory inside a Lua-facing function unless you also set up a `__gc` metamethod to free it. For shapes, the scene owns the memory — Lua only holds a raw pointer.

---

## Submitting a pull request

1. **One PR per concern.** A PR that fixes a bug and adds a feature is two PRs.
2. **Describe the change** in the PR description — what problem it solves and how.
3. **Test manually** by running the default `scripts/scene.lua` and any scene that exercises your change.
4. **Keep history clean.** Squash fixup commits before marking the PR ready for review.
5. If your change affects the public API or Lua API, update `README.md` accordingly.

---

## Reporting bugs

Open an issue and include:

- OS and compiler version (`g++ --version` or `clang++ --version`)
- CMake version (`cmake --version`)
- Exact error message or description of the unexpected behavior
- Minimal reproduction — a short Lua script or C++ snippet that triggers the issue

---

Happy coding!
