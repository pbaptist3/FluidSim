#ifndef FLUIDSIM_RENDER_H
#define FLUIDSIM_RENDER_H

#include <vector>
#include "Particle.h"
#include <string>

/// Stores relevant information about particle OpenGL state
struct GLRenderInfo {
    unsigned p_vao;
    unsigned p_vbo;
    unsigned p_prog;
    unsigned tex;
    unsigned p_fbo;
    unsigned p_rbo;
};

/// Renders all particles to a texture and return the texture id
unsigned int render_particles(const std::vector<Particle>& particles, GLRenderInfo info, int width, int height);

/// Load a program for OpenGL to use
void load_shader(const std::string& filename);

/// Initialize particle rendering with OpenGL
GLRenderInfo init_render();

#endif
