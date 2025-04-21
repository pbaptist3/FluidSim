#ifndef FLUIDSIM_BINARYPARTITIONCONTAINER_H
#define FLUIDSIM_BINARYPARTITIONCONTAINER_H

#include "ParticleContainer.h"
#include <vector>

class BinaryPartitionContainer : public ParticleContainer
{
public:
    struct Node {
        Node* q[4]; // 4 quadrants
        std::vector<int> d; // indices of particles
    };

    class Iterator {
        BinaryPartitionContainer& c;
        Particle& p;
        Iterator(BinaryPartitionContainer& c, Particle& p);
    public:
        virtual Iterator& operator++();
        Particle& operator*();
        bool done();
        friend BinaryPartitionContainer;
        int idx();
    };

    /// Rehashes all particles
    void update(float r);

private:
    // root node
    Node* root;
};

#endif
