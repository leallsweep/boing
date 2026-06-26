#pragma once
// ─── boink_texture.h ─────────────────────────────────────────────────────────
// Texture loading (PNG, JPG, BMP, TGA) via stb_image.
// Usage:
//   auto* tex = TextureCache::load("path/to/image.png");
//   shape->setTexture(tex);
//   shape->setTexture(nullptr);  // remove texture, use flat color instead
// ─────────────────────────────────────────────────────────────────────────────

#include <string>
#include <unordered_map>
#include <memory>

namespace Boink {

// ─── Texture ─────────────────────────────────────────────────────────────────
struct Texture {
    unsigned int id     = 0;  // OpenGL texture handle
    int          width  = 0;
    int          height = 0;
    int          channels = 0;
    std::string  path;

    ~Texture();  // calls glDeleteTextures
};

// ─── TextureCache ─────────────────────────────────────────────────────────────
// Loads each file once; returns the same Texture* for repeated paths.
class TextureCache {
public:
    // Load (or retrieve cached) texture. Returns nullptr on failure.
    static Texture* load(const std::string& path);

    // Release all GPU textures (call on shutdown).
    static void clear();

private:
    static std::unordered_map<std::string, std::unique_ptr<Texture>> s_cache;
};

} // namespace Boink
