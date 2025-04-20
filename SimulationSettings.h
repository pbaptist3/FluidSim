#pragma once

struct SimulationSettings {
    float gravity = 9.8f;
    float viscosity = 0.1f;
    float timestep = 0.01f;
    bool paused = false;
};