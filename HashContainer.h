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
    class Iterator {
        int i;
        int offset;
        float radius;
        HashContainer& c;
        Particle& p;
        Iterator(HashContainer& c, Particle& p, float r);
    public:
        virtual Iterator& operator++();
        Particle& operator*();
        bool done();
        friend HashContainer;
        int idx();
    };

    /// Rehashes all particles
    void update(float r);
};


#endif
