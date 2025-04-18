#include "ParticleContainer.h"

ParticleContainer::~ParticleContainer() {}

std::vector<Particle>& ParticleContainer::vec()
{
    return this->particles;
}