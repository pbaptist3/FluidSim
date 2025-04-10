#ifndef FLUIDSIM_PARTICLECONTAINER_H
#define FLUIDSIM_PARTICLECONTAINER_H

#include <vector>
#include "Particle.h"

// ABC for a generic particle container
class ParticleContainer
{
    std::vector<Particle> particles;

public:
    virtual ~ParticleContainer() = 0;
};


#endif
