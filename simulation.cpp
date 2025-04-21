#include "simulation.h"

/// Perform a physics update on all particles
///
/// 1. duplicate particle container
/// 2. calculate densities
/// 3. calculate pressure gradient
/// 4. calculate viscosity
/// 5. apply all forces
/// 6. apply velocity
void Simulation::phys_update()
{
    particles.update(smoothing_radius);
    std::vector<Particle> out(particles.vec());

    for (int i=0; i<out.size(); i++)
    {
        Particle& p = get_particles().vec()[i];
        Particle& p_out = out[i];

        p_out.px += p.vx * timestep;
        p_out.py += p.vy * timestep;
        p_out.vy -= gravity * timestep;

        // bounds checks
        if (p_out.px > 1.0)
        {
            p_out.px = 1.0;
            p_out.vx *= -0.5;
            p_out.vy *= 0.5;
        }
        if (p_out.px < -1.0)
        {
            p_out.px = -1.0;
            p_out.vx *= -0.5;
            p_out.vy *= 0.5;
        }
        if (p_out.py > 1.0)
        {
            p_out.py = 1.0;
            p_out.vy *= -0.5;
            p_out.vx *= 0.5;
        }
        if (p_out.py < -1.0)
        {
            p_out.py = -1.0;
            p_out.vy *= -0.5;
            p_out.vx *= 0.5;
        }

        // give a nudge away from floor
        if (p_out.py < -0.98)
        {
            //p_out.vy += 2 * gravity * timestep;
        }
    }

    // calculate densities and pressures
    std::vector<float> densities(out.size(), 0.0);
    std::vector<float> pressures(out.size(), 0.0);
    for (int i=0; i<out.size(); i++)
    {
        Particle& p1 = particles.vec()[i];

        for (auto p2 = particles.nearest(i, smoothing_radius); !p2.done(); ++p2)
        {
            if (p1.px == (*p2).px && p1.py == (*p2).py) continue;
            densities[i] += mass * kernel(p1, *p2);
        }
        pressures[i] = gas_constant / 10000.0 * (target_density - densities[i]);
    }

    // calculate pressure and viscosity forces
    for (int i=0; i<out.size(); i++)
    {
        Particle& p1 = particles.vec()[i];
        std::pair<float, float> force = {0.0, 0.0};

        for (auto p2 = particles.nearest(i, smoothing_radius); !p2.done(); ++p2)
        {
            if (p1.px == (*p2).px && p1.py == (*p2).py) continue;
            auto gradient = kernel_gradient(p1, *p2);
            float componentless = 0.0;
            if (densities[p2.idx()] != 0.0)
                componentless = (pressures[i] + pressures[p2.idx()]) * mass * -0.5 / densities[p2.idx()];
            force.first += componentless * gradient.first;
            force.second += componentless * gradient.second;

            componentless = viscosity / 1000000.0 * 0.5 * kernel_laplacian(p1, *p2);
            force.first += ((*p2).vx - p1.vx) * componentless;
            force.second += ((*p2).vy - p1.vy) * componentless;
        }

        Particle& p1out = out[i];
        p1out.vx += timestep * force.first;
        p1out.vy += timestep * force.second;
    }

    particles.vec() = out;
}

ParticleContainer &Simulation::get_particles()
{
    return particles;
}

float Simulation::kernel(const Particle &p1, const Particle &p2)
{
    float dx = p2.px - p1.px, dy = p2.py - p1.py;
    float r_sq = dx*dx + dy*dy;
    float r_sm_sq = smoothing_radius*smoothing_radius;
    float diff_sq = r_sm_sq - r_sq;
    return diff_sq*diff_sq*diff_sq / (r_sm_sq*r_sm_sq*r_sm_sq*r_sm_sq*r_sm_sq);
}

std::pair<float, float> Simulation::kernel_gradient(const Particle &p1, const Particle &p2)
{
    float dx = p2.px - p1.px, dy = p2.py - p1.py;
    float r_sq = dx*dx + dy*dy;
    float r_sm_sq = smoothing_radius*smoothing_radius;
    float diff_sq = r_sm_sq - r_sq;
    float componentless_part = -6.0 * diff_sq*diff_sq / (r_sm_sq*r_sm_sq*r_sm_sq*r_sm_sq*r_sm_sq);
    return {componentless_part * dx, componentless_part * dy};
}

float Simulation::kernel_laplacian(const Particle &p1, const Particle &p2)
{
    float dx = p1.px - p2.px, dy = p1.py - p2.py;
    float r_sq = dx*dx + dy*dy;
    float r_sm_sq = smoothing_radius*smoothing_radius;
    float diff_sq = r_sm_sq - r_sq;
    float componentless_part = 6.0 * diff_sq / (r_sm_sq*r_sm_sq*r_sm_sq*r_sm_sq*r_sm_sq);
    return componentless_part * (6.0*r_sq - 2.0*r_sm_sq);
}
