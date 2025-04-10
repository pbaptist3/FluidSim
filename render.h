#ifndef FLUIDSIM_RENDER_H
#define FLUIDSIM_RENDER_H

#include <vector>
#include "Particle.h"

/// Renders all particles to a texture and return the texture id
unsigned int render_particles(const std::vector<Particle>& particles);

#endif
