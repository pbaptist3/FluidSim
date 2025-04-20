#ifndef FLUIDSIM_PARTICLECONTAINER_H
#define FLUIDSIM_PARTICLECONTAINER_H

#include <vector>
#include "Particle.h"

// A generic particle container
class ParticleContainer
{
    protected:
    std::vector<Particle> particles;

public:
    class Iterator {
        int i;
        float radius;
        ParticleContainer& c;
        Particle& p;
        Iterator(ParticleContainer& c, Particle& p, float r) : i(0), c(c), p(p), radius(r) {}
    public:
        virtual Iterator& operator++();
        Particle& operator*();
        bool done();
        friend ParticleContainer;
        int idx();
    };

    std::vector<Particle>& vec();
    virtual void insert(Particle p);
    virtual ParticleContainer::Iterator nearest(int i, float r);
    virtual void update(float r) {}
};


#endif
