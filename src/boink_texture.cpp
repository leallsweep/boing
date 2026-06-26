#include "../include/boink_texture.h"
#include <GL/glew.h>
#include <cstdio>

// Compile stb_image here (once)
#define STB_IMAGE_IMPLEMENTATION
#define STBI_FAILURE_USERMSG
#include "../include/stb_image.h"

namespace Boink {

// ─── Texture destructor ───────────────────────────────────────────────────────
Texture::~Texture() {
    if(id) glDeleteTextures(1, &id);
}

// ─── TextureCache ─────────────────────────────────────────────────────────────
std::unordered_map<std::string, std::unique_ptr<Texture>> TextureCache::s_cache;

Texture* TextureCache::load(const std::string& path) {
    // Return cached handle if already loaded
    auto it = s_cache.find(path);
    if(it != s_cache.end()) return it->second.get();

    // Load pixels with stb_image (flip vertically for OpenGL)
    stbi_set_flip_vertically_on_load(true);
    int w, h, ch;
    unsigned char* data = stbi_load(path.c_str(), &w, &h, &ch, 0);
    if(!data) {
        fprintf(stderr, "[Boink] Texture load failed: %s — %s\n",
                path.c_str(), stbi_failure_reason());
        return nullptr;
    }

    // Choose format
    GLenum fmt = GL_RGB;
    if(ch == 1) fmt = GL_RED;
    if(ch == 4) fmt = GL_RGBA;

    // Upload to GPU
    unsigned int texID = 0;
    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D, texID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,     GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,     GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, (GLint)fmt, w, h, 0, fmt, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, 0);
    stbi_image_free(data);

    // Store in cache
    auto tex       = std::make_unique<Texture>();
    tex->id        = texID;
    tex->width     = w;
    tex->height    = h;
    tex->channels  = ch;
    tex->path      = path;

    Texture* raw = tex.get();
    s_cache[path] = std::move(tex);

    fprintf(stdout, "[Boink] Texture loaded: %s (%dx%d ch=%d)\n",
            path.c_str(), w, h, ch);
    return raw;
}

void TextureCache::clear() {
    s_cache.clear(); // destructors call glDeleteTextures
}

} // namespace Boink
