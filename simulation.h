#ifndef FLUIDSIM_SIMULATION_H
#define FLUIDSIM_SIMULATION_H

#include <vector>
#include <array>
#include "particle.h"
#include "ParticleContainer.h"
#include "HashContainer.h"
#include "BinaryPartitionContainer.h"

class Simulation {
    HashContainer particles;

    /// Calculate the kernel between two particles
    float kernel(const Particle& p1, const Particle& p2);

    /// Calculate gradient of the kernel between two particles
    std::pair<float, float> kernel_gradient(const Particle& p1, const Particle& p2);

    /// Calculate laplacian of the kernel between two particles
    float kernel_laplacian(const Particle& p1, const Particle& p2);

public:
    enum class Pattern {
        Grid,
        Circle,
        Random
    };

    Simulation() : smoothing_radius(0.15), timestep(0.005), gravity(1.0), gas_constant(0.02), viscosity(0.0),
        target_density(6000.0), mass(1.0), spawn_pattern(Pattern::Grid), paused(true), particle_count(1000) {}

    /// Perform a physics update on all particles
    void phys_update();

    ParticleContainer& get_particles();

    // These fields are public so the imgui sliders can access them more easily
    float smoothing_radius;
    float timestep;
    float gravity;
    float gas_constant;
    float viscosity;
    float target_density;
    float mass;
    Pattern spawn_pattern;
    bool paused;
    int particle_count;
};

#endif
