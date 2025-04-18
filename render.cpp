#include "render.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include "glad/gl.h"
#include <GLFW/glfw3.h>
#include "shaders.h"

unsigned render_particles(const std::vector<Particle>& particles, GLRenderInfo info, int width, int height)
{
    glUseProgram(info.p_prog);
    glBindVertexArray(info.p_vao);

    glBindTexture(GL_TEXTURE_2D, info.tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, info.tex, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, info.p_fbo);
    glBindRenderbuffer(GL_RENDERBUFFER, info.p_rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, info.p_rbo);

    glBindBuffer(GL_ARRAY_BUFFER, info.p_vbo);
    glBufferData(GL_ARRAY_BUFFER, particles.size()*sizeof(Particle), particles.data(), GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (void*)0);
    glEnableVertexAttribArray(0);

    GLenum draw_buffers[1] = {GL_COLOR_ATTACHMENT0};
    glDrawBuffers(1, draw_buffers);
    glBindFramebuffer(GL_FRAMEBUFFER, info.tex);
    glViewport(0, 0, width, height);
    glDrawArrays(GL_POINTS, 0, particles.size());

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return info.tex;
}

unsigned load_shader(const char** shader_source, int type)
{
    unsigned int shader_id = glCreateShader(type);
    glShaderSource(shader_id, 1, shader_source, NULL);
    glCompileShader(shader_id);

    char error_msg[512];
    int result;
    glGetShaderiv(shader_id, GL_COMPILE_STATUS, &result);
    if (!result)
    {
        glGetShaderInfoLog(shader_id, 512, NULL, error_msg);
        std::cerr << "Error in compiling shader: " << error_msg << std::endl;
    }

    return shader_id;
}

GLRenderInfo init_render()
{
    GLRenderInfo info;

    gladLoaderLoadGL();

    unsigned vao;
    glGenVertexArrays(1, &vao);
    info.p_vao = vao;

    unsigned p_buf;
    glGenBuffers(1, &p_buf);
    glBindBuffer(GL_ARRAY_BUFFER, p_buf);
    info.p_vbo = p_buf;
    int vert = load_shader(&P_VERT_STR, GL_VERTEX_SHADER);
    int frag = load_shader(&P_FRAG_STR, GL_FRAGMENT_SHADER);
    unsigned prog = glCreateProgram();
    glAttachShader(prog, vert);
    glAttachShader(prog, frag);
    glLinkProgram(prog);
    info.p_prog = prog;

    glDeleteShader(vert);
    glDeleteShader(frag);

    glEnable(GL_PROGRAM_POINT_SIZE);
    glPointSize(2.0);

    unsigned framebuf;
    glGenFramebuffers(1, &framebuf);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuf);
    info.p_fbo = framebuf;

    unsigned tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);
    info.tex = tex;

    unsigned rbo;
    glGenRenderbuffers(1, &rbo);
    info.p_rbo = rbo;

    return info;
}