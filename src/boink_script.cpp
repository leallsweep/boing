#include "../include/boink_script.h"
#include <lua.hpp>
#include <cstdio>
#include <cstring>

// ─── Lua helper macros ───────────────────────────────────────────────────────
#define LUA_CHECK(L, cond, msg) \
    if(!(cond)){ lua_pushstring(L, msg); lua_error(L); }

namespace Boink {

// Scene pointer stored as light userdata in Lua registry
static const char* SCENE_KEY = "__boink_scene";

static Scene* getScene(lua_State* L){
    lua_getfield(L, LUA_REGISTRYINDEX, SCENE_KEY);
    Scene* s = static_cast<Scene*>(lua_touserdata(L, -1));
    lua_pop(L,1);
    return s;
}

// ─── Shape userdata helpers ──────────────────────────────────────────────────
static const char* SHAPE_MT = "Boink.Shape";

static Shape** pushShape(lua_State* L, Shape* s){
    Shape** ud = (Shape**)lua_newuserdata(L, sizeof(Shape*));
    *ud = s;
    luaL_getmetatable(L, SHAPE_MT);
    lua_setmetatable(L, -2);
    return ud;
}
static Shape* checkShape(lua_State* L, int idx){
    Shape** ud = (Shape**)luaL_checkudata(L, idx, SHAPE_MT);
    LUA_CHECK(L, ud && *ud, "invalid shape");
    return *ud;
}

// ─── Shape methods ────────────────────────────────────────────────────────────
static int shape_moveTo(lua_State* L){
    Shape* s=checkShape(L,1);
    s->moveTo((float)luaL_checknumber(L,2),
              (float)luaL_checknumber(L,3),
              (float)luaL_checknumber(L,4));
    lua_pushvalue(L,1); return 1;
}
static int shape_move(lua_State* L){
    Shape* s=checkShape(L,1);
    s->move((float)luaL_checknumber(L,2),
            (float)luaL_checknumber(L,3),
            (float)luaL_checknumber(L,4));
    lua_pushvalue(L,1); return 1;
}
static int shape_rotateTo(lua_State* L){
    Shape* s=checkShape(L,1);
    s->rotateTo((float)luaL_checknumber(L,2),
                (float)luaL_checknumber(L,3),
                (float)luaL_checknumber(L,4));
    lua_pushvalue(L,1); return 1;
}
static int shape_rotate(lua_State* L){
    Shape* s=checkShape(L,1);
    s->rotate((float)luaL_checknumber(L,2),
              (float)luaL_checknumber(L,3),
              (float)luaL_checknumber(L,4));
    lua_pushvalue(L,1); return 1;
}
static int shape_scaleTo(lua_State* L){
    Shape* s=checkShape(L,1);
    s->scaleTo((float)luaL_checknumber(L,2),
               (float)luaL_checknumber(L,3),
               (float)luaL_checknumber(L,4));
    lua_pushvalue(L,1); return 1;
}
static int shape_setColor(lua_State* L){
    Shape* s=checkShape(L,1);
    float a = lua_isnumber(L,5) ? (float)lua_tonumber(L,5) : 1.0f;
    s->setColor((float)luaL_checknumber(L,2),
                (float)luaL_checknumber(L,3),
                (float)luaL_checknumber(L,4), a);
    lua_pushvalue(L,1); return 1;
}
static int shape_setWireframe(lua_State* L){
    checkShape(L,1)->setWireframe(lua_toboolean(L,2));
    lua_pushvalue(L,1); return 1;
}
static int shape_getName(lua_State* L){
    lua_pushstring(L, checkShape(L,1)->name.c_str()); return 1;
}
static int shape_getType(lua_State* L){
    lua_pushstring(L, checkShape(L,1)->type().c_str()); return 1;
}
static int shape_getPosition(lua_State* L){
    Vec3 p=checkShape(L,1)->position;
    lua_pushnumber(L,p.x); lua_pushnumber(L,p.y); lua_pushnumber(L,p.z);
    return 3;
}

static const luaL_Reg shape_methods[] = {
    {"moveTo",      shape_moveTo},
    {"move",        shape_move},
    {"rotateTo",    shape_rotateTo},
    {"rotate",      shape_rotate},
    {"scaleTo",     shape_scaleTo},
    {"setColor",    shape_setColor},
    {"setWireframe",shape_setWireframe},
    {"getName",     shape_getName},
    {"getType",     shape_getType},
    {"getPosition", shape_getPosition},
    {nullptr, nullptr}
};

// ─── Scene (global) Lua API ──────────────────────────────────────────────────
static int lua_addCube(lua_State* L){
    Scene* sc = getScene(L);
    const char* name = luaL_checkstring(L,1);
    pushShape(L, sc->addCube(name)); return 1;
}
static int lua_addPlane(lua_State* L){
    Scene* sc = getScene(L);
    const char* name = luaL_checkstring(L,1);
    int sub = lua_isnumber(L,2) ? (int)lua_tointeger(L,2) : 1;
    pushShape(L, sc->addPlane(name, sub)); return 1;
}
static int lua_addTriangle(lua_State* L){
    Scene* sc = getScene(L);
    const char* name = luaL_checkstring(L,1);
    pushShape(L, sc->addTriangle(name)); return 1;
}
static int lua_getShape(lua_State* L){
    Scene* sc = getScene(L);
    const char* name = luaL_checkstring(L,1);
    Shape* s = sc->get(name);
    if(s) pushShape(L,s);
    else  lua_pushnil(L);
    return 1;
}
static int lua_removeShape(lua_State* L){
    Scene* sc = getScene(L);
    const char* name = luaL_checkstring(L,1);
    lua_pushboolean(L, sc->remove(name)); return 1;
}

// ── Camera ────────────────────────────────────────────────────────────────────
static int lua_cameraMoveTo(lua_State* L){
    getScene(L)->cameraMoveTo(
        (float)luaL_checknumber(L,1),
        (float)luaL_checknumber(L,2),
        (float)luaL_checknumber(L,3));
    return 0;
}
static int lua_cameraMove(lua_State* L){
    getScene(L)->cameraMove(
        (float)luaL_checknumber(L,1),
        (float)luaL_checknumber(L,2),
        (float)luaL_checknumber(L,3));
    return 0;
}
static int lua_cameraLookAt(lua_State* L){
    getScene(L)->cameraLookAt(
        (float)luaL_checknumber(L,1),
        (float)luaL_checknumber(L,2),
        (float)luaL_checknumber(L,3));
    return 0;
}
static int lua_cameraSetFov(lua_State* L){
    getScene(L)->cameraSetFov((float)luaL_checknumber(L,1));
    return 0;
}
static int lua_cameraGetPosition(lua_State* L){
    Vec3 p=getScene(L)->camera.position;
    lua_pushnumber(L,p.x); lua_pushnumber(L,p.y); lua_pushnumber(L,p.z);
    return 3;
}

// ── Utility ───────────────────────────────────────────────────────────────────
static int lua_print(lua_State* L){
    int n = lua_gettop(L);
    for(int i=1;i<=n;i++){
        if(i>1) printf("\t");
        printf("%s", luaL_tolstring(L,i,nullptr));
        lua_pop(L,1);
    }
    printf("\n");
    return 0;
}

// ─── ScriptEngine ────────────────────────────────────────────────────────────
ScriptEngine::ScriptEngine() {
    L = luaL_newstate();
    luaL_openlibs(L);
}
ScriptEngine::~ScriptEngine() {
    if(L) lua_close(L);
}

void ScriptEngine::bindScene(Scene* scene) {
    m_scene = scene;
    // Store scene pointer in registry
    lua_pushlightuserdata(L, scene);
    lua_setfield(L, LUA_REGISTRYINDEX, SCENE_KEY);
    registerAPI();
}

void ScriptEngine::registerAPI() {
    // Create Shape metatable
    luaL_newmetatable(L, SHAPE_MT);
    lua_pushvalue(L,-1); lua_setfield(L,-2,"__index");
    luaL_setfuncs(L, shape_methods, 0);
    lua_pop(L,1);

    // Register global functions in "Boink" table
    lua_newtable(L);

    auto reg = [&](const char* name, lua_CFunction fn){
        lua_pushcfunction(L, fn);
        lua_setfield(L,-2, name);
    };
    reg("addCube",           lua_addCube);
    reg("addPlane",          lua_addPlane);
    reg("addTriangle",       lua_addTriangle);
    reg("getShape",          lua_getShape);
    reg("removeShape",       lua_removeShape);
    reg("cameraMoveTo",      lua_cameraMoveTo);
    reg("cameraMove",        lua_cameraMove);
    reg("cameraLookAt",      lua_cameraLookAt);
    reg("cameraSetFov",      lua_cameraSetFov);
    reg("cameraGetPosition", lua_cameraGetPosition);

    lua_setglobal(L, "Boink");

    // override print
    lua_pushcfunction(L, lua_print);
    lua_setglobal(L, "print");
}

bool ScriptEngine::execFile(const std::string& path, std::string& err) {
    if(luaL_dofile(L, path.c_str())){
        err = lua_tostring(L,-1); lua_pop(L,1);
        return false;
    }
    return true;
}

bool ScriptEngine::execString(const std::string& code, std::string& err) {
    if(luaL_dostring(L, code.c_str())){
        err = lua_tostring(L,-1); lua_pop(L,1);
        return false;
    }
    return true;
}

bool ScriptEngine::callFunction(const std::string& name, float dt, std::string& err) {
    lua_getglobal(L, name.c_str());
    if(!lua_isfunction(L,-1)){ lua_pop(L,1); return true; } // not required
    lua_pushnumber(L, (double)dt);
    if(lua_pcall(L,1,0,0)){
        err = lua_tostring(L,-1); lua_pop(L,1);
        return false;
    }
    return true;
}

} // namespace Boink
