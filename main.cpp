//  ╔══════════════════════════════════════════════════════╗
//  ║               B O I N K   E N G I N E              ║
//  ║  Usage:  ./boink [script.lua]                       ║
//  ║  Default script: scripts/scene.lua                  ║
//  ╚══════════════════════════════════════════════════════╝
//
//  Controls (built-in):
//    WASD        — move camera forward/back/strafe
//    Q / E       — move camera up / down
//    Arrow keys  — orbit around camera target
//    Escape      — quit
//
//  From C++ you can also set a per-frame callback without Lua:
//
//    window.setUpdateCallback([](Boink::Scene& scene, float dt){
//        auto* cube = scene.get("myCube");
//        if(cube) cube->rotate(0, dt, 0);
//    });

#include "boink.h"
#include <cstdio>
#include <string>

int main(int argc, char** argv) {
    // ── Window configuration ──────────────────────────────────────────────
    Boink::WindowConfig cfg;
    cfg.width  = 1280;
    cfg.height = 720;
    cfg.title  = "Boink";
    cfg.vsync  = true;

    Boink::Window window(cfg);

    // ── Choose script ─────────────────────────────────────────────────────
    std::string script = (argc > 1) ? argv[1] : "scripts/scene.lua";
    if(!window.loadScript(script)){
        fprintf(stderr, "[Boink] Could not load script '%s'.\n"
                        "        Falling back to default C++ scene.\n",
                        script.c_str());

        // Fallback: build a minimal scene in C++ directly
        auto& scene = window.scene();
        scene.cameraMoveTo(0, 2, 6);
        scene.cameraLookAt(0, 0, 0);

        auto* ground = scene.addPlane("ground");
        ground->scaleTo(8,1,8);
        ground->moveTo(0,-1,0);
        ground->setColor(0.3f, 0.5f, 0.3f);

        auto* cube = scene.addCube("cube");
        cube->setColor(0.2f, 0.6f, 1.0f);

        auto* tri = scene.addTriangle("tri");
        tri->moveTo(-2, 0, 0);
        tri->setColor(1.0f, 0.8f, 0.1f);

        window.setUpdateCallback([](Boink::Scene& s, float dt){
            static float t = 0; t += dt;
            auto* c = s.get("cube");
            if(c) c->rotateTo(t*0.5f, t, 0);
            auto* tr = s.get("tri");
            if(tr) tr->rotateTo(0, t*1.5f, 0);
        });
    }

    // ── Main loop ─────────────────────────────────────────────────────────
    printf("[Boink] Starting. Press Esc to exit.\n");
    window.run();
    printf("[Boink] Bye!\n");
    return 0;
}
