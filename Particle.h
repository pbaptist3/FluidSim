#ifndef FLUIDSIM_PARTICLE_H
#define FLUIDSIM_PARTICLE_H

/// Represents a single particle in 3D
struct Particle
{
    float px;
    float py;
    float pz;
    float vx;
    float vy;
    float vz;

    /// Makes a particle at specified position at rest
    Particle(float px, float py, float pz)
    : px(px), py(py), pz(pz), vx(0.0f), vy(0.0f), vz(0.0f) {}

    //Hash function for grid size
    int hash(float grid_size);
};

#endif
