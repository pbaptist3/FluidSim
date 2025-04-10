#ifndef FLUIDSIM_SIMULATION_H
#define FLUIDSIM_SIMULATION_H

#include <vector>
#include <array>
#include "particle.h"
#include "ParticleContainer.h"

/// Calculate gradient of a scalar field
std::vector<std::array<float, 3>> calculate_gradient(const std::vector<float>& field);

/// Calculate laplacian of a vector field
std::vector<float> calculate_laplacian(const std::vector<std::array<float, 3>> field);

/// Calculate desnity of particles using neighbors
std::vector<float> calculate_density(const ParticleContainer &particles, float radius);

/// Perform a physics update on all particles
void phys_update(const ParticleContainer &particles, float dt);

#endif
