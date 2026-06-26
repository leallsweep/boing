#pragma once
#include "boink_scene.h"
#include <string>

// Forward-declare Lua state to avoid pulling lua headers everywhere
struct lua_State;

namespace Boink {

class ScriptEngine {
public:
    ScriptEngine();
    ~ScriptEngine();

    // Bind a scene so all Lua calls operate on it
    void bindScene(Scene* scene);

    // Execute a Lua file
    bool execFile(const std::string& path, std::string& errOut);

    // Execute a Lua string
    bool execString(const std::string& code, std::string& errOut);

    // Call a global Lua function (e.g. "onUpdate")
    bool callFunction(const std::string& name, float dt, std::string& errOut);

    lua_State* L = nullptr;

private:
    Scene* m_scene = nullptr;
    void registerAPI();
};

} // namespace Boink
