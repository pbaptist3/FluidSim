#include "ParticleContainer.h"

#include <math.h>

std::vector<Particle>& ParticleContainer::vec()
{
    return this->particles;
}

ParticleContainer::Iterator ParticleContainer::nearest(int i, float radius)
{
    return Iterator(*this, particles[i], radius);
}

void ParticleContainer::insert(Particle p)
{
    particles.push_back(p);
}

ParticleContainer::Iterator& ParticleContainer::Iterator::operator++()
{
    while (i < (int)c.vec().size())
    {
        ++i;
        float dx = c.vec()[i].px - p.px;
        float dy = c.vec()[i].py - p.py;
        if ((dx*dx+dy*dy)<(radius*radius))
            break;
    }
    return *this;
}

Particle& ParticleContainer::Iterator::operator*()
{
    return c.vec()[i];
}

bool ParticleContainer::Iterator::done()
{
    return i >= c.vec().size();
}

int ParticleContainer::Iterator::idx()
{
    return i;
}