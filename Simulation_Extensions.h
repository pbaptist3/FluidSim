#pragma once
#include "HashContainer.h"
#include "SimulationSettings.h"

inline void apply_extended_physics(HashContainer& particles, const SimulationSettings& settings) {
    for (auto& p : particles.vec()) {
        // Apply gravity in Y
        p.vy -= settings.gravity * settings.timestep;

        // Apply viscosity damping
        p.vx *= (1.0f - settings.viscosity * settings.timestep);
        p.vy *= (1.0f - settings.viscosity * settings.timestep);
        p.vz *= (1.0f - settings.viscosity * settings.timestep);

        // Position update
        p.px += p.vx * settings.timestep;
        p.py += p.vy * settings.timestep;
        p.pz += p.vz * settings.timestep;
    }
}
