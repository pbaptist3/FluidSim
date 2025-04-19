#ifndef FLUIDSIM_HASHCONTAINER_H
#define FLUIDSIM_HASHCONTAINER_H

#include <vector>
#include <array>
#include "ParticleContainer.h"

/// A particle container using a hash to group nearby particles
class HashContainer : public ParticleContainer
{
    // buckets of indices
    // -1 indicates unused
    std::vector<int> buckets;

public:
    ~HashContainer() {};

    /// Recompute all particles' hashes
    void rehash();

    /// Insert a single particle into the container
    void insert(Particle p);
};


#endif
