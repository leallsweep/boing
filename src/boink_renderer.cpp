#include "../include/boink_renderer.h"
#include <GL/glew.h>
#include <cstdio>
#include <cstring>

namespace Boink {

// ─── Shaders ─────────────────────────────────────────────────────────────────
static const char* VERT_SRC = R"glsl(
#version 330 core
layout(location=0) in vec3 aPos;
layout(location=1) in vec3 aNormal;

uniform mat4 uMVP;
uniform mat4 uModel;

out vec3 vNormal;
out vec3 vFragPos;

void main(){
    vec4 world = uModel * vec4(aPos, 1.0);
    vFragPos   = world.xyz;
    vNormal    = mat3(transpose(inverse(uModel))) * aNormal;
    gl_Position = uMVP * vec4(aPos, 1.0);
}
)glsl";

static const char* FRAG_SRC = R"glsl(
#version 330 core
in  vec3 vNormal;
in  vec3 vFragPos;
out vec4 FragColor;

uniform vec4  uColor;
uniform vec3  uLightDir;
uniform float uAmbient;

void main(){
    vec3  n     = normalize(vNormal);
    float diff  = max(dot(n, normalize(uLightDir)), 0.0);
    float light = uAmbient + (1.0 - uAmbient) * diff;
    FragColor   = vec4(uColor.rgb * light, uColor.a);
}
)glsl";

// ─── Renderer ────────────────────────────────────────────────────────────────
unsigned int Renderer::compileShader(unsigned int type, const char* src) {
    unsigned int id = glCreateShader(type);
    glShaderSource(id, 1, &src, nullptr);
    glCompileShader(id);
    int ok; glGetShaderiv(id, GL_COMPILE_STATUS, &ok);
    if(!ok){
        char log[512]; glGetShaderInfoLog(id,512,nullptr,log);
        fprintf(stderr, "[Boink] Shader error: %s\n", log);
    }
    return id;
}

bool Renderer::init(int /*width*/, int /*height*/) {
    glewExperimental = GL_TRUE;
    if(glewInit() != GLEW_OK){
        fprintf(stderr,"[Boink] glewInit failed\n");
        return false;
    }
    unsigned int vs = compileShader(GL_VERTEX_SHADER,   VERT_SRC);
    unsigned int fs = compileShader(GL_FRAGMENT_SHADER, FRAG_SRC);
    m_shaderProg = glCreateProgram();
    glAttachShader(m_shaderProg, vs);
    glAttachShader(m_shaderProg, fs);
    glLinkProgram(m_shaderProg);
    glDeleteShader(vs); glDeleteShader(fs);

    m_uMVP      = glGetUniformLocation(m_shaderProg, "uMVP");
    m_uModel    = glGetUniformLocation(m_shaderProg, "uModel");
    m_uColor    = glGetUniformLocation(m_shaderProg, "uColor");
    m_uLightDir = glGetUniformLocation(m_shaderProg, "uLightDir");
    m_uAmbient  = glGetUniformLocation(m_shaderProg, "uAmbient");

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    return true;
}

void Renderer::resize(int w, int h) {
    glViewport(0,0,w,h);
}

void Renderer::clear(float r, float g, float b) {
    glClearColor(r, g, b, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::uploadShape(Shape* s) {
    if(s->vao) {
        glDeleteVertexArrays(1,&s->vao);
        glDeleteBuffers(1,&s->vbo);
        glDeleteBuffers(1,&s->ebo);
    }
    glGenVertexArrays(1,&s->vao);
    glGenBuffers(1,&s->vbo);
    glGenBuffers(1,&s->ebo);

    glBindVertexArray(s->vao);

    glBindBuffer(GL_ARRAY_BUFFER, s->vbo);
    glBufferData(GL_ARRAY_BUFFER,
                 s->vertices.size()*sizeof(float),
                 s->vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, s->ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 s->indices.size()*sizeof(unsigned int),
                 s->indices.data(), GL_STATIC_DRAW);

    // position
    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,6*sizeof(float),(void*)0);
    glEnableVertexAttribArray(0);
    // normal
    glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,6*sizeof(float),(void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
    s->indexCount = (int)s->indices.size();
}

// Helper: multiply two Mat4
static Mat4 mul(const Mat4& a, const Mat4& b) { return a * b; }

void Renderer::draw(Shape* s, const Camera& cam, int vpW, int vpH) {
    if(!s->vao) return;
    float aspect = vpW > 0 && vpH > 0 ? (float)vpW/(float)vpH : 1.0f;

    Mat4 model = s->modelMatrix();
    Mat4 view  = cam.viewMatrix();
    Mat4 proj  = cam.projectionMatrix(aspect);
    Mat4 mvp   = mul(proj, mul(view, model));

    glUseProgram(m_shaderProg);
    glUniformMatrix4fv(m_uMVP,   1, GL_FALSE, mvp.data());
    glUniformMatrix4fv(m_uModel, 1, GL_FALSE, model.data());
    glUniform4f(m_uColor, s->color.r, s->color.g, s->color.b, s->color.a);
    glUniform3f(m_uLightDir, 1.0f, 2.0f, 1.5f);
    glUniform1f(m_uAmbient, 0.25f);

    glBindVertexArray(s->vao);
    if(s->wireframe) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glDrawElements(GL_TRIANGLES, s->indexCount, GL_UNSIGNED_INT, nullptr);
    if(s->wireframe) glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glBindVertexArray(0);
}

void Renderer::drawAll(const std::vector<std::shared_ptr<Shape>>& shapes,
                       const Camera& cam, int vpW, int vpH) {
    for(auto& s : shapes) draw(s.get(), cam, vpW, vpH);
}

} // namespace Boink
