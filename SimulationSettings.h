#pragma once

struct SimulationSettings {
    float gravity = 9.8f;
    float viscosity = 0.1f;
    float timestep = 0.01f;
    bool paused = false;

    int particle_count = 100;
    float particle_radius = 0.01f;

    enum class Pattern {
        Grid,
        Circle,
        Random
    };
    Pattern spawn_pattern = Pattern::Grid;

};